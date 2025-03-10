/**
 * @file CanDriver.cpp
 * @author Simon Cahill (contact@simonc.eu)
 * @brief
 * @version 0.1
 * @date 2020-07-01
 *
 * @copyright Copyright (c) 2020 Simon Cahill
 *
 *
 *  Copyright 2020 Simon Cahill
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
// stl
#include <array>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

// libc
#include <fcntl.h>
#include <ifaddrs.h>
#include <linux/can.h>
#include <linux/can/netlink.h>
#include <linux/can/raw.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// mnl
#include <libmnl/libmnl.h>

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

    using std::array;
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

    bool CanDriver::setInterfaceUp(const string& interface, const size_t bitrate) {
        if (interface.empty()) { throw CanInitException("Interface name cannot be empty!"); }

        auto flags = 0;
        flags |= IFF_UP;
        time_t sequence = time(nullptr);

        char mnlBuffer[MNL_SOCKET_BUFFER_SIZE];
        memset(mnlBuffer, 0, MNL_SOCKET_BUFFER_SIZE);

        auto nlHeader = mnl_nlmsg_put_header(mnlBuffer);
        nlHeader->nlmsg_type = RTM_NEWLINK;
        nlHeader->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
        nlHeader->nlmsg_seq = sequence;

        auto ifaceInfoMsg = static_cast<ifinfomsg*>(mnl_nlmsg_put_extra_header(nlHeader, sizeof(ifinfomsg)));
        ifaceInfoMsg->ifi_family = AF_CAN;
        ifaceInfoMsg->ifi_change = flags;
        ifaceInfoMsg->ifi_flags = flags;

        mnl_attr_put_str(nlHeader, IFLA_IFNAME, interface.c_str());

        auto canInfo = mnl_attr_nest_start(nlHeader, IFLA_LINKINFO);
        mnl_attr_put_str(nlHeader, IFLA_INFO_KIND, "can");

        auto canData = mnl_attr_nest_start(nlHeader, IFLA_INFO_DATA);
        mnl_attr_put_u32(nlHeader, IFLA_CAN_BITTIMING, bitrate);

        mnl_attr_nest_end(nlHeader, canData);

        auto nlSocket = mnl_socket_open(NETLINK_ROUTE);
        if (nlSocket == nullptr) { throw CanInitException("FAILED to open netlink socket!"); }

        if (mnl_socket_bind(nlSocket, 0, MNL_SOCKET_AUTOPID) == -1) {
            mnl_socket_close(nlSocket);
            throw CanInitException("FAILED to bind netlink socket!");
        }

        const auto portId = mnl_socket_get_portid(nlSocket);

        #ifdef sockcanpp_DEBUG
        mnl_nlmsg_fprintf(stdout, nlHeader, nlHeader->nlmsg_len, sizeof(ifinfomsg));
        #endif

        if (mnl_socket_sendto(nlSocket, nlHeader, nlHeader->nlmsg_len) == -1) {
            mnl_socket_close(nlSocket);
            throw CanInitException("FAILED to send netlink message!");
        }

        auto nlResponse = mnl_socket_recvfrom(nlSocket, mnlBuffer, MNL_SOCKET_BUFFER_SIZE);

        if (nlResponse == -1) {
            mnl_socket_close(nlSocket);
            throw CanInitException("FAILED to receive netlink response!");
        }

        nlResponse = mnl_cb_run(mnlBuffer, nlResponse, sequence, portId, nullptr, nullptr);

        if (nlResponse == -1 && errno != 0) {
            mnl_socket_close(nlSocket);
            throw CanInitException("FAILED to run netlink callback! Error: " + string{strerror(errno)});
        }

        mnl_socket_close(nlSocket);

        return nlResponse == MNL_CB_OK;
    }

    vector<string> CanDriver::getAvailableInterfaces() {
        vector<string> interfaces{};

        ifaddrs* ifaceList{};
        ifaddrs* iface{};

        if (getifaddrs(&ifaceList) < 0) { throw CanException("FAILED to get interface list!", "*"); }

        iface = ifaceList;

        do {
            if ((iface->ifa_addr && iface->ifa_addr->sa_family == AF_CAN) || string{iface->ifa_name}.find("can") != string::npos) {
                interfaces.push_back(iface->ifa_name);
            }
        } while ((iface = iface->ifa_next) != nullptr);

        return interfaces;
    }
        

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
     * @param timeout The time (in millis) to wait before timing out.
     *
     * @return true If messages are available on the bus.
     * @return false Otherwise.
     */
    bool CanDriver::waitForMessages(milliseconds timeout) {
        if (_socketFd < 0) { throw InvalidSocketException("Invalid socket!", _socketFd); }

        unique_lock<mutex> locky(_lock);

        fd_set readFileDescriptors;
        timeval waitTime;
        waitTime.tv_sec = timeout.count() / 1000;
        waitTime.tv_usec = timeout.count() * 1000;

        FD_ZERO(&readFileDescriptors);
        FD_SET(_socketFd, &readFileDescriptors);
        const auto fdsAvailable = select(_socketFd + 1, &readFileDescriptors, nullptr, nullptr, &waitTime);

        int32_t bytesAvailable{0};
        const auto retCode = ioctl(_socketFd, FIONREAD, &bytesAvailable);
        if (retCode == 0) {
            _queueSize = static_cast<int32_t>(std::ceil(bytesAvailable / sizeof(can_frame)));
        } else {
            _queueSize = 0;
        }

        return fdsAvailable > 0;
    }

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

        memset(&canFrame, 0, sizeof(can_frame));

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
    ssize_t CanDriver::sendMessageQueue(queue<CanMessage> messages, milliseconds delay, bool forceExtended) {
        if (_socketFd < 0) { throw InvalidSocketException("Invalid socket!", _socketFd); }

        ssize_t totalBytesWritten = 0;

        while (!messages.empty()) {
            totalBytesWritten += sendMessage(messages.front(), forceExtended);
            messages.pop();
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

        for (int32_t i = _queueSize; 0 < i; --i) {
	        messages.emplace(readMessageLock(false));
        }

        return messages;
    }

    /**
     * @brief Attempts to set the filter mask for the associated CAN bus.
     *
     * @param mask The bit mask to apply.
     * @param filterId The ID to filter on.
     */
    void CanDriver::setCanFilterMask(const int32_t mask, const CanId& filterId) {
        setCanFilters({{filterId, static_cast<uint32_t>(mask)}});
    }

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
            canFilters.push_back({id, filter});
        }
        #else
        for (const auto& filterPair : filters) {
            canFilters.push_back({filterPair.first, filterPair.second});
        }
        #endif

        if (setsockopt(_socketFd, SOL_CAN_RAW, CAN_RAW_FILTER, canFilters.data(), canFilters.size() * sizeof(can_filter)) == -1) {
            throw CanInitException(formatString("FAILED to set CAN filters on socket %d! Error: %d => %s", _socketFd, errno, strerror(errno)));
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
        // unique_lock<mutex> locky(_lock);

        struct sockaddr_can address;
        struct ifreq ifaceRequest;
        int64_t fdOptions = 0;
        int32_t tmpReturn;

        memset(&address, 0, sizeof(struct sockaddr_can));
        memset(&ifaceRequest, 0, sizeof(struct ifreq));

        _socketFd = socket(PF_CAN, SOCK_RAW, _canProtocol);

        if (_socketFd == -1) {
            throw CanInitException(formatString("FAILED to initialise socketcan! Error: %d => %s", errno, strerror(errno)));
        }

        strcpy(ifaceRequest.ifr_name, _canInterface.c_str());

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
