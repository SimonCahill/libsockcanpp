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
    using std::strncpy;
    using std::unique_lock;
    using std::unique_ptr;
    using std::chrono::milliseconds;
    using std::this_thread::sleep_for;
    using std::vector;

    //////////////////////////////////////
    //      PUBLIC IMPLEMENTATION       //
    //////////////////////////////////////


#pragma region "Object Construction"
    CanDriver::CanDriver(const string& canInterface, int32_t canProtocol, const CanId defaultSenderId):
        CanDriver(canInterface, canProtocol, 0 /* match all */, defaultSenderId) { }

    CanDriver::CanDriver(const string& canInterface, const int32_t canProtocol, const int32_t filterMask, const CanId defaultSenderId):
        CanDriver(canInterface, canProtocol, filtermap_t{{0, filterMask}}, defaultSenderId) { }

    CanDriver::CanDriver(const string& canInterface, const int32_t canProtocol, const filtermap_t& filters, const CanId defaultSenderId):
        _defaultSenderId(defaultSenderId), _canFilterMask(filters), _canProtocol(canProtocol), _canInterface(canInterface) {
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
        if (_socketFd < 0) { throw InvalidSocketException("Invalid socket!", _socketFd); }

        unique_lock<mutex> locky(_lock);

        fd_set readFileDescriptors;
        timeval waitTime{0, static_cast<suseconds_t>(timeout.count())};

        FD_ZERO(&readFileDescriptors);
        FD_SET(_socketFd, &readFileDescriptors);
        const auto fdsAvailable = select(_socketFd + 1, &readFileDescriptors, nullptr, nullptr, &waitTime);

        int32_t bytesAvailable{0};
        const auto retCode = ioctl(_socketFd, FIONREAD, &bytesAvailable);
        if (retCode == 0) {
            _queueSize = static_cast<int32_t>(std::ceil(bytesAvailable / sizeof(can_frame)));
        } else {
            _queueSize = 0;
            // vcan interfaces don't support FIONREAD. So fall back to
            // using alternate implementation in readQueuedMessages().
            _canReadQueueSize = false;
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
    CanMessage CanDriver::readMessageLock(bool const lock) {
        unique_ptr<unique_lock<mutex>> locky{nullptr};

        if (lock) { locky = unique_ptr<unique_lock<mutex>>{new unique_lock<mutex>{_lock}}; }
        
        if (0 > _socketFd) { throw InvalidSocketException("Invalid socket!", _socketFd); }

        ssize_t readBytes{0};
        can_frame canFrame{};

        readBytes = read(_socketFd, &canFrame, sizeof(can_frame));

        if (0 > readBytes) { throw CanException(formatString("FAILED to read from CAN! Error: %d => %s", errno, strerror(errno)), _socketFd); }

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
        if (_socketFd < 0) { throw InvalidSocketException("Invalid socket!", _socketFd); }

        unique_lock<mutex> locky(_lockSend);

        ssize_t bytesWritten = 0;

        if (message.getFrameData().size() > CAN_MAX_DATA_LENGTH) {
            throw CanException(formatString("INVALID data length! Message must be smaller than %d bytes!", CAN_MAX_DATA_LENGTH), _socketFd);
        }

        auto canFrame = message.getRawFrame();

        if (forceExtended || ((uint32_t)message.getCanId() > CAN_SFF_MASK)) { canFrame.can_id |= CAN_EFF_FLAG; }

        bytesWritten = write(_socketFd, (const void*)&canFrame, sizeof(canFrame));

        if (bytesWritten == -1) { throw CanException(formatString("FAILED to write data to socket! Error: %d => %s", errno, strerror(errno)), _socketFd); }

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
        if (_socketFd < 0) { throw InvalidSocketException("Invalid socket!", _socketFd); }

        ssize_t totalBytesWritten = 0;

        while (!messages.empty()) {
            totalBytesWritten += sendMessage(messages.front(), forceExtended);
            messages.pop();

            if (delay.count() > 0) {
                sleep_for(delay);
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
        if (_socketFd < 0) { throw InvalidSocketException("Invalid socket!", _socketFd); }

        unique_lock<mutex> locky{_lock};
        queue<CanMessage> messages{};

        if (_canReadQueueSize) {
            for (int32_t i = _queueSize; 0 < i; --i) {
                messages.emplace(readMessageLock(false));
            }
        } else {
            // If the interface doesn't support FIONREAD, fall back
            // to reading until data exhausted.
            bool more{true};

            do {
                ssize_t readBytes;
                can_frame canFrame{};
                readBytes = read(_socketFd, &canFrame, sizeof(can_frame));
                if (readBytes >= 0) {
                    messages.emplace(canFrame);
                } else if (errno == EAGAIN) {
                    more = false;
                } else {
                    throw CanException(formatString("FAILED to read from CAN! Error: %d => %s", errno, strerror(errno)), _socketFd);
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
        if (_socketFd < 0) { throw InvalidSocketException("Invalid socket!", _socketFd); }

        int32_t canFdFrames = enabled ? 1 : 0;

        if (setsockopt(_socketFd, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &canFdFrames, sizeof(canFdFrames)) == -1) {
            throw CanInitException(formatString("FAILED to set CAN FD frames on socket %d! Error: %d => %s", _socketFd, errno, strerror(errno)));
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
        if (_socketFd < 0) { throw InvalidSocketException("Invalid socket!", _socketFd); }

        int32_t canXlFrames = enabled ? 1 : 0;

        
        if (setsockopt(_socketFd, SOL_CAN_RAW, CAN_RAW_XL_FRAMES, &canXlFrames, sizeof(canXlFrames)) == -1) {
            throw CanInitException(formatString("FAILED to set CAN XL frames on socket %d! Error: %d => %s", _socketFd, errno, strerror(errno)));
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
        if (_socketFd < 0) { throw InvalidSocketException("Invalid socket!", _socketFd); }

        int32_t joinFilters = 1;

        if (setsockopt(_socketFd, SOL_CAN_RAW, CAN_RAW_JOIN_FILTERS, &joinFilters, sizeof(joinFilters)) == -1) {
            throw CanInitException(formatString("FAILED to join CAN filters on socket %d! Error: %d => %s", _socketFd, errno, strerror(errno)));
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
        if (_socketFd < 0) { throw InvalidSocketException("Invalid socket!", _socketFd); }

        unique_lock<mutex> locky(_lock);
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

        if (setsockopt(_socketFd, SOL_CAN_RAW, CAN_RAW_FILTER, canFilters.data(), canFilters.size() * sizeof(can_filter)) == -1) {
            throw CanInitException(formatString("FAILED to set CAN filters on socket %d! Error: %d => %s", _socketFd, errno, strerror(errno)));
        }
    }

    /**
     * @brief Sets the error filter for the associated CAN bus.
     *
     * @param enabled Whether or not to enable the error filter.
     */
    void CanDriver::setErrorFilter(const bool enabled/* = true*/) const {
        if (_socketFd < 0) { throw InvalidSocketException("Invalid socket!", _socketFd); }

        int32_t errorFilter = enabled ? 1 : 0;

        if (setsockopt(_socketFd, SOL_CAN_RAW, CAN_RAW_ERR_FILTER, &errorFilter, sizeof(errorFilter)) == -1) {
            throw CanInitException(formatString("FAILED to set CAN error filter on socket %d! Error: %d => %s", _socketFd, errno, strerror(errno)));
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
        if (_socketFd < 0) { throw InvalidSocketException("Invalid socket!", _socketFd); }

        int32_t receiveOwnMessages = enabled ? 1 : 0;

        if (setsockopt(_socketFd, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &receiveOwnMessages, sizeof(receiveOwnMessages)) == -1) {
            throw CanInitException(formatString("FAILED to set CAN error filter on socket %d! Error: %d => %s", _socketFd, errno, strerror(errno)));
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
        int32_t tmpReturn{0};

        _socketFd = socket(PF_CAN, SOCK_RAW, _canProtocol);

        if (_socketFd == -1) {
            throw CanInitException(formatString("FAILED to initialise socketcan! Error: %d => %s", errno, strerror(errno)));
        }

        std::copy(_canInterface.begin(), _canInterface.end(), ifaceRequest.ifr_name);

        if ((tmpReturn = ioctl(_socketFd, SIOCGIFINDEX, &ifaceRequest)) == -1) {
            throw CanInitException(formatString("FAILED to perform IO control operation on socket %s! Error: %d => %s", _canInterface.c_str(), errno,
                                    strerror(errno)));
        }

        fdOptions = fcntl(_socketFd, F_GETFL);
        fdOptions |= O_NONBLOCK;
        tmpReturn = fcntl(_socketFd, F_SETFL, fdOptions);

        address.can_family = AF_CAN;
        address.can_ifindex = ifaceRequest.ifr_ifindex;

        setCanFilters(_canFilterMask);

        if ((tmpReturn = bind(_socketFd, (struct sockaddr*)&address, sizeof(address))) == -1) {
            throw CanInitException(formatString("FAILED to bind to socket CAN! Error: %d => %s", errno, strerror(errno)));
        }
    }

    /**
     * @brief Closes the underlying CAN socket.
     */
    void CanDriver::uninitialiseSocketCan() {
        unique_lock<mutex> locky(_lock);

        if (_socketFd <= 0) { throw CanCloseException("Cannot close invalid socket!"); }

        if (close(_socketFd) == -1) { throw CanCloseException(formatString("FAILED to close CAN socket! Error: %d => %s", errno, strerror(errno))); }

        _socketFd = -1;
    }
#pragma endregion

} // namespace sockcanpp
