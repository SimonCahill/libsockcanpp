/**
 * @file CanDriver.cpp
 * @author Simon Cahill (contact@simonc.eu)
 * @brief
 * @version 0.1
 * @date 2020-07-01
 *
 * @copyright Copyright (c) 2020-2025 Simon Cahill
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

//////////////////////////////
//      SYSTEM INCLUDES     //
//////////////////////////////

#include <fcntl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

//////////////////////////////
//      LOCAL  INCLUDES     //
//////////////////////////////
#include "CanDriver.hpp"
#include "CanId.hpp"
#include "CanMessage.hpp"
#include "exceptions/CanCloseException.hpp"
#include "exceptions/CanException.hpp"
#include "exceptions/CanInitException.hpp"
#include "exceptions/InvalidSocketException.hpp"

namespace sockcanpp {

    using exceptions::CanCloseException;
    using exceptions::CanException;
    using exceptions::CanInitException;
    using exceptions::InvalidSocketException;

    using std::mutex;
    using std::queue;
    using std::string;
    using std::unique_lock;
    using std::unique_ptr;
    using std::chrono::milliseconds;
    using std::vector;

    namespace thread = std::this_thread;

    //////////////////////////////////////
    //      PUBLIC IMPLEMENTATION       //
    //////////////////////////////////////


#pragma region "Object Construction"
    CanDriver::CanDriver(const string& canInterface, int32_t canProtocol, const CanId defaultSenderId):
        CanDriver(canInterface, canProtocol, 0 /* match all */, defaultSenderId) { }

    CanDriver::CanDriver(const string& canInterface, const int32_t canProtocol, const int32_t filterMask, const CanId defaultSenderId):
        CanDriver(canInterface, canProtocol, filtermap_t{{0, filterMask}}, defaultSenderId) { }

    CanDriver::CanDriver(const string& canInterface, const int32_t canProtocol, const filtermap_t& filters, const CanId defaultSenderId):
        m_defaultSenderId(defaultSenderId), m_canFilterMask(filters), m_canProtocol(canProtocol), m_canInterface(canInterface) {
        initialiseSocketCan();
    }
#pragma endregion

#pragma region "I / O"
    /**
     * @brief Blocks until one or more CAN messages appear on the bus, or until the timeout runs out.
     *
     * @param timeout The time (in µsec) to wait before timing out.
     *
     * @return true If messages are available on the bus.
     * @return false Otherwise.
     */
    bool CanDriver::waitForMessages(microseconds timeout/* = microseconds(3000)*/) {
        if (m_socketFd < 0) { throw InvalidSocketException("Invalid socket!", m_socketFd); }

        unique_lock<mutex> locky(m_lock);

        fd_set readFileDescriptors;
        timeval waitTime{0, static_cast<suseconds_t>(timeout.count())};

        FD_ZERO(&readFileDescriptors);
        FD_SET(m_socketFd, &readFileDescriptors);
        const auto fdsAvailable = select(m_socketFd + 1, &readFileDescriptors, nullptr, nullptr, &waitTime);

        int32_t bytesAvailable{0};
        const auto retCode = ioctl(m_socketFd, FIONREAD, &bytesAvailable);
        if (retCode == 0) {
            m_queueSize = static_cast<int32_t>(std::ceil(bytesAvailable / sizeof(can_frame)));
        } else {
            m_queueSize = 0;
            // vcan interfaces don't support FIONREAD. So fall back to
            // using alternate implementation in readQueuedMessages().
            m_canReadQueueSize = false;
        }

        return fdsAvailable > 0;
    }

    /**
     * @brief Blocks until one or more CAN messages appear on the bus, or until the timeout runs out.
     *
     * @param timeout The time (in millis) to wait before timing out.
     *
     * @return true If messages are available on the bus.
     * @return false Otherwise.
     */
    bool CanDriver::waitForMessages(milliseconds timeout) { return waitForMessages(std::chrono::duration_cast<microseconds>(timeout)); }

    /**
     * @brief Blocks until one or more CAN messages appear on the bus, or until the timeout runs out.
     *
     * @param timeout The time (in nanoseconds) to wait before timing out.
     *
     * @return true If messages are available on the bus.
     * @return false Otherwise.
     */
    bool CanDriver::waitForMessages(nanoseconds timeout/* = nanoseconds(3000)*/) { return waitForMessages(std::chrono::duration_cast<microseconds>(timeout)); }

    /**
     * @brief Attempts to read a message from the associated CAN bus.
     *
     * @return CanMessage The message read from the bus.
     */
    CanMessage CanDriver::readMessage() { return readMessageLock(); }

    /**
     * @brief readMessage deadlock guard, attempts to read a message from the associated CAN bus.
     *
     * @return CanMessage The message read from the bus.
     */
    CanMessage CanDriver::readMessageLock(bool const lock /* = true */) {
        unique_ptr<unique_lock<mutex>> locky{nullptr};

        if (lock) { locky = unique_ptr<unique_lock<mutex>>{new unique_lock<mutex>{m_lock}}; }
        
        if (0 > m_socketFd) { throw InvalidSocketException("Invalid socket!", m_socketFd); }
        can_frame canFrame{};

        if (read(m_socketFd, &canFrame, sizeof(can_frame)) < 0) {
            throw CanException(
                #if __cpp_lib_format < 202002L
                formatString("FAILED to read from CAN! Error: %d => %s", errno, strerror(errno))
                #else
                std::format("FAILED to read from CAN! Error: {0:d} => {1:s}", errno, strerror(errno))
                #endif // __cpp_lib_format < 202002L
            , m_socketFd);
        }

        if (m_collectTelemetry) {
            // Read timestamp from the socket if available.
            struct timeval tv{};
            if (ioctl(m_socketFd, SIOCGSTAMP, &tv) < 0) {
                throw CanException(
                    #if __cpp_lib_format < 202002L
                    formatString("FAILED to read timestamp from socket! Error: %d => %s", errno, strerror(errno))
                    #else
                    std::format("FAILED to read timestamp from socket! Error: {0:d} => {1:s}", errno, strerror(errno))
                    #endif // __cpp_lib_format < 202002L
                , m_socketFd);
            }
            return CanMessage{canFrame, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::seconds(tv.tv_sec) + std::chrono::microseconds(tv.tv_usec))};
        }

        return CanMessage{canFrame};
    }
    
    /**
     * @brief Attempts to send a CAN message on the associated bus.
     *
     * @param message The message to be sent.
     * @param forceExtended Whether or not to force use of an extended ID.
     *
     * @return ssize_t The amount of bytes sent on the bus.
     */
    ssize_t CanDriver::sendMessage(const CanMessage& message, bool forceExtended) {
        if (m_socketFd < 0) { throw InvalidSocketException("Invalid socket!", m_socketFd); }

        unique_lock<mutex> locky(m_lockSend);

        ssize_t bytesWritten = 0;

        if (message.getFrameData().size() > CAN_MAX_DATA_LENGTH) {
            throw CanException(
                #if __cpp_lib_format < 202002L
                formatString("INVALID data length! Message must be smaller than %d bytes!", CAN_MAX_DATA_LENGTH)
                #else
                std::format("INVALID data length! Message must be smaller than {0:d} bytes!", CAN_MAX_DATA_LENGTH)
                #endif // __cpp_lib_format < 202002L
            , m_socketFd);
        }

        auto canFrame = message.getRawFrame();

        if (forceExtended || ((uint32_t)message.getCanId() > CAN_SFF_MASK)) { canFrame.can_id |= CAN_EFF_FLAG; }

        bytesWritten = write(m_socketFd, &canFrame, sizeof(canFrame));

        if (bytesWritten < 0) {
            throw CanException(
                #if __cpp_lib_format < 202002L
                formatString("FAILED to write data to socket! Error: %d => %s", errno, strerror(errno))
                #else
                std::format("FAILED to write data to socket! Error: {0:d} => {1:s}", errno, strerror(errno))
                #endif // __cpp_lib_format < 202002L
            , m_socketFd);
        }

        return bytesWritten;
    }

    /**
     * @brief Attempts to send a queue of messages on the associated CAN bus.
     *
     * @param messages A queue containing the messages to be sent.
     * @param delay If greater than 0, will delay the sending of the next message.
     * @param forceExtended Whether or not to force use of an extended ID.
     *
     * @return int32_t The total amount of bytes sent.
     */
    ssize_t CanDriver::sendMessageQueue(queue<CanMessage>& messages, microseconds delay, bool forceExtended) { return sendMessageQueue(messages, std::chrono::duration_cast<nanoseconds>(delay), forceExtended); }

    /**
     * @brief Attempts to send a queue of messages on the associated CAN bus.
     *
     * @param messages A queue containing the messages to be sent.
     * @param delay If greater than 0, will delay the sending of the next message.
     * @param forceExtended Whether or not to force use of an extended ID.
     *
     * @return int32_t The total amount of bytes sent.
     */
    ssize_t CanDriver::sendMessageQueue(queue<CanMessage>& messages, milliseconds delay, bool forceExtended) { return sendMessageQueue(messages, std::chrono::duration_cast<nanoseconds>(delay), forceExtended); }

    /**
     * @brief Attempts to send a queue of messages on the associated CAN bus.
     *
     * @param messages A queue containing the messages to be sent.
     * @param delay If greater than 0, will delay the sending of the next message.
     * @param forceExtended Whether or not to force use of an extended ID.
     *
     * @return int32_t The total amount of bytes sent.
     */
    ssize_t CanDriver::sendMessageQueue(queue<CanMessage>& messages, nanoseconds delay, bool forceExtended) {
        if (m_socketFd < 0) { throw InvalidSocketException("Invalid socket!", m_socketFd); }

        ssize_t totalBytesWritten = 0;

        while (!messages.empty()) {
            totalBytesWritten += sendMessage(messages.front(), forceExtended);
            messages.pop();

            if (delay.count() > 0) {
                thread::sleep_for(delay);
            }
        }

        return totalBytesWritten;
    }

    /**
     * @brief Attempts to read all messages stored in the buffer for the associated CAN bus.
     *
     * @return queue<CanMessage> A queue containing the messages read from the bus buffer.
     */
    queue<CanMessage> CanDriver::readQueuedMessages() {
        if (m_socketFd < 0) { throw InvalidSocketException("Invalid socket!", m_socketFd); }

        unique_lock<mutex> locky{m_lock};
        queue<CanMessage> messages{};

        if (m_canReadQueueSize) {
            for (int32_t i = m_queueSize; 0 < i; --i) {
                messages.emplace(readMessageLock(false));
            }
        } else {
            // If the interface doesn't support FIONREAD, fall back
            // to reading until data exhausted.
            bool more{true};

            do {
                ssize_t readBytes;
                can_frame canFrame{};
                readBytes = read(m_socketFd, &canFrame, sizeof(can_frame));
                if (readBytes >= 0) {
                    if (m_collectTelemetry) {
                        // Read timestamp from the socket if available.
                        struct timeval tv{};
                        if (ioctl(m_socketFd, SIOCGSTAMP, &tv) < 0) {
                            throw CanException(
                                #if __cpp_lib_format < 202002L
                                formatString("FAILED to read timestamp from socket! Error: %d => %s", errno, strerror(errno))
                                #else
                                std::format("FAILED to read timestamp from socket! Error: {0:d} => {1:s}", errno, strerror(errno))
                                #endif // __cpp_lib_format < 202002L
                            , m_socketFd);
                        }
                        messages.emplace(CanMessage{canFrame, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::seconds(tv.tv_sec) + std::chrono::microseconds(tv.tv_usec))});
                    } else {
                        messages.emplace(canFrame);
                    }
                } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    more = false;
                } else {
                    throw CanException(formatString("FAILED to read from CAN! Error: %d => %s", errno, strerror(errno)), m_socketFd);
                }
            } while (more);
        }

        return messages;
    }

    /**
     * @brief Sets the CAN FD frame option for the interface.
     * 
     * This option allows the current driver instance to receive CAN FD frames.
     * 
     * @param enabled Whether or not to enable the CAN FD frame option.
     */
    void CanDriver::allowCanFdFrames(const bool enabled/* = true*/) const {
        if (m_socketFd < 0) { throw InvalidSocketException("Invalid socket!", m_socketFd); }

        int32_t canFdFrames = enabled ? 1 : 0;

        if (setsockopt(m_socketFd, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &canFdFrames, sizeof(canFdFrames)) == -1) {
            throw CanInitException(formatString("FAILED to set CAN FD frames on socket %d! Error: %d => %s", m_socketFd, errno, strerror(errno)));
        }
    }

    #ifdef CANXL_XLF
    /**
     * @brief Sets the CAN XL option for the interface.
     * 
     * This option allows the current driver instance to send/receive CAN XL frames.
     * 
     * @param enabled Whether or not to enable the CAN XL option.
     */
    void CanDriver::allowCanXlFrames(const bool enabled/* = true*/) const {
        if (m_socketFd < 0) { throw InvalidSocketException("Invalid socket!", m_socketFd); }

        int32_t canXlFrames = enabled ? 1 : 0;

        
        if (setsockopt(m_socketFd, SOL_CAN_RAW, CAN_RAW_XL_FRAMES, &canXlFrames, sizeof(canXlFrames)) == -1) {
            throw CanInitException(formatString("FAILED to set CAN XL frames on socket %d! Error: %d => %s", m_socketFd, errno, strerror(errno)));
        }
    }
    #endif // CANXL_XLF

    /**
     * @brief Configures the socket to join the CAN filters.
     * This is especially required, when using inverted CAN filters.
     * 
     * Source: https://stackoverflow.com/a/57680496/2921426
     */
    void CanDriver::joinCanFilters() const {
        if (m_socketFd < 0) { throw InvalidSocketException("Invalid socket!", m_socketFd); }

        int32_t joinFilters = 1;

        if (setsockopt(m_socketFd, SOL_CAN_RAW, CAN_RAW_JOIN_FILTERS, &joinFilters, sizeof(joinFilters)) == -1) {
            throw CanInitException(formatString("FAILED to join CAN filters on socket %d! Error: %d => %s", m_socketFd, errno, strerror(errno)));
        }
    }

    /**
     * @brief Attempts to set the filter mask for the associated CAN bus.
     *
     * @param mask The bit mask to apply.
     * @param filterId The ID to filter on.
     */
    void CanDriver::setCanFilterMask(const int32_t mask, const CanId& filterId) { setCanFilters({{filterId, static_cast<uint32_t>(mask)}}); }

    /**
     * @brief Sets multiple CAN filters for the associated CAN bus.
     *
     * @param filters A map containing the filters to apply.
     */
    void CanDriver::setCanFilters(const filtermap_t& filters) {
        if (m_socketFd < 0) { throw InvalidSocketException("Invalid socket!", m_socketFd); }

        unique_lock<mutex> locky(m_lock);
        vector<can_filter> canFilters{};

        // Structured bindings only available with C++17
        #if __cplusplus >= 201703L
        for (const auto [id, filter] : filters) {
            canFilters.push_back({*id, filter});
        }
        #else
        for (const auto& filterPair : filters) {
            canFilters.push_back({*filterPair.first, filterPair.second});
        }
        #endif

        if (setsockopt(m_socketFd, SOL_CAN_RAW, CAN_RAW_FILTER, canFilters.data(), canFilters.size() * sizeof(can_filter)) == -1) {
            throw CanInitException(formatString("FAILED to set CAN filters on socket %d! Error: %d => %s", m_socketFd, errno, strerror(errno)));
        }
    }

    /**
     * @brief Enables collection of advanced telemetry for the associated CAN bus.
     *
     * @param enabled Whether or not to enable telemetry collection.
     */
    void CanDriver::setCollectTelemetry(const bool enabled/* = true*/) { m_collectTelemetry = enabled; }

    /**
     * @brief Sets the error filter for the associated CAN bus.
     *
     * @param enabled Whether or not to enable the error filter.
     */
    void CanDriver::setErrorFilter(const bool enabled/* = true*/) const {
        if (m_socketFd < 0) { throw InvalidSocketException("Invalid socket!", m_socketFd); }

        can_err_mask_t errorMask{enabled ? CAN_ERR_MASK : 0x00};

        if (setsockopt(m_socketFd, SOL_CAN_RAW, CAN_RAW_ERR_FILTER, &errorMask, sizeof(can_err_mask_t)) == -1) {
            throw CanInitException(
                #if __cpp_lib_format < 202002L
                formatString("FAILED to set CAN error filter on socket %d! Error: %d => %s", m_socketFd, errno, strerror(errno))
                #else
                std::format("FAILED to set CAN error filter on socket {0:d}! Error: {1:d} => {2:s}", m_socketFd, errno, strerror(errno))
                #endif // __cpp_lib_format < 202002L
            );
        }
    }
    
    /**
     * @brief Sets the receive own messages option for the associated CAN bus.
     * 
     * This option allows the socket to receive its own messages.
     * 
     * @param enabled Whether or not to enable the receive own messages option.
     */
    void CanDriver::setReceiveOwnMessages(const bool enabled) const {
        if (m_socketFd < 0) { throw InvalidSocketException("Invalid socket!", m_socketFd); }

        int32_t receiveOwnMessages = enabled ? 1 : 0;

        if (setsockopt(m_socketFd, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &receiveOwnMessages, sizeof(receiveOwnMessages)) == -1) {
            throw CanInitException(
                #if __cpp_lib_format < 202002L
                formatString("FAILED to set CAN message echo on socket %d! Error: %d => %s", m_socketFd, errno, strerror(errno))
                #else
                std::format("FAILED to set CAN message echo on socket {0:d}! Error: {1:d} => {2:s}", m_socketFd, errno, strerror(errno))
                #endif // __cpp_lib_format < 202002L
            );
        }
    }
#pragma endregion

    //////////////////////////////////////
    //      PROTECTED IMPLEMENTATION    //
    //////////////////////////////////////

#pragma region Socket Management

    /**
     * @brief Initialises the underlying CAN socket.
     */
    void CanDriver::initialiseSocketCan() {
        struct sockaddr_can address{};
        struct ifreq ifaceRequest{};
        int64_t fdOptions{0};

        m_socketFd = socket(PF_CAN, SOCK_RAW, m_canProtocol);

        if (m_socketFd == -1) {
            throw CanInitException(
                #if __cpp_lib_format < 202002L
                formatString("FAILED to initialise socketcan! Error: %d => %s", errno, strerror(errno))
                #else
                std::format("FAILED to initialise socketcan! Error: {0:d} => {1:s}", errno, strerror(errno))
                #endif // __cpp_lib_format < 202002L
            );
        }

        std::copy(m_canInterface.begin(), m_canInterface.end(), ifaceRequest.ifr_name);

        if (ioctl(m_socketFd, SIOCGIFINDEX, &ifaceRequest) < 0) {
            throw CanInitException(
                #if __cpp_lib_format < 202002L
                formatString("FAILED to perform IO control operation on socket %s! Error: %d => %s", m_canInterface.c_str(), errno,
                                    strerror(errno))
                #else
                std::format("FAILED to perform IO control operation on socket {0:s}! Error: {1:d} => {2:s}", m_canInterface, errno, strerror(errno))
                #endif // __cpp_lib_format < 202002L
            );
        }

        fdOptions = fcntl(m_socketFd, F_GETFL);
        fdOptions |= O_NONBLOCK;
        if (fcntl(m_socketFd, F_SETFL, fdOptions) < 0) {
            throw CanInitException(
                #if __cpp_lib_format < 202002L
                formatString("FAILED to set non-blocking mode on socket %s! Error: %d => %s", m_canInterface.c_str(), errno, strerror(errno))
                #else
                std::format("FAILED to set non-blocking mode on socket {0:s}! Error: {1:d} => {2:s}", m_canInterface, errno, strerror(errno))
                #endif // __cpp_lib_format < 202002L
            );
        }

        address.can_family = AF_CAN;
        address.can_ifindex = ifaceRequest.ifr_ifindex;

        setCanFilters(m_canFilterMask);

        if (bind(m_socketFd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
            throw CanInitException(
                #if __cpp_lib_format < 202002L
                formatString("FAILED to bind to socket CAN! Error: %d => %s", errno, strerror(errno))
                #else
                std::format("FAILED to bind to socket CAN! Error: {0:d} => {1:s}", errno, strerror(errno))
                #endif // __cpp_lib_format < 202002L
            );
        }
    }

    /**
     * @brief Closes the underlying CAN socket.
     */
    void CanDriver::uninitialiseSocketCan() {
        unique_lock<mutex> locky(m_lock);

        if (m_socketFd <= 0) { throw CanCloseException("Cannot close invalid socket!"); }

        if (close(m_socketFd) == -1) {
            throw CanCloseException(
                #if __cpp_lib_format < 202002L
                formatString("FAILED to close CAN socket! Error: %d => %s", errno, strerror(errno))
                #else
                std::format("FAILED to close CAN socket! Error: {0:d} => {1:s}", errno, strerror(errno))
                #endif // __cpp_lib_format < 202002L
            );
        }

        m_socketFd = -1;
    }
#pragma endregion

} // namespace sockcanpp
