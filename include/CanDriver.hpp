/**
 * @file CanDriver.hpp
 * @author Simon Cahill (contact@simonc.eu)
 * @brief Contains the declarations for the SocketCAN wrapper in C++.
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

#ifndef LIBSOCKCANPP_INCLUDE_CANDRIVER_HPP
#define LIBSOCKCANPP_INCLUDE_CANDRIVER_HPP

//////////////////////////////
//      SYSTEM INCLUDES     //
//////////////////////////////
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>

#if __cplusplus >= 201703L
#include <optional>
#endif // __cplusplus >= 201703L

//////////////////////////////
//      LOCAL  INCLUDES     //
//////////////////////////////
#include "CanId.hpp"
#include "CanMessage.hpp"

#if __cplusplus < 201703L
#include "TlOptional.hpp"
#endif // __cplusplus < 201703L

/**
 * @brief Main library namespace.
 * 
 * This namespace contains the library's main code.
 */
namespace sockcanpp {

    #if __cplusplus >= 201300
    using namespace std::chrono_literals;
    #endif // __cplusplus >= 201300

    #if __cplusplus >= 201703L
    using std::optional;
    #else
    using tl::optional;
    #endif // __cplusplus < 201703L

    using std::chrono::microseconds;
    using std::chrono::milliseconds;
    using std::chrono::nanoseconds;
    using std::mutex;
    using std::string;
    using std::queue;
    using std::unordered_map;

    using filtermap_t = unordered_map<CanId, uint32_t, CanIdHasher>;

    /**
     * @brief CanDriver class; handles communication via CAN.
     * 
     * This class provides the means of easily communicating with other devices via CAN in C++.
     * 
     * @remarks
     * This class may be inherited by other applications and modified to suit your needs.
     */
    class CanDriver {
        public: // +++ Static +++
            static constexpr int32_t CAN_MAX_DATA_LENGTH = 8; //!< The maximum amount of bytes allowed in a single CAN frame
            static constexpr int32_t CAN_SOCK_RAW        = CAN_RAW; //!< The raw CAN protocol
            static constexpr int32_t CAN_SOCK_SEVEN      = 7; //!< A separate CAN protocol, used by certain embedded device OEMs.

        public: // +++ Constructor / Destructor +++
            CanDriver(const string& canInterface, const int32_t canProtocol, const CanId defaultSenderId = 0); //!< Constructor
            CanDriver(const string& canInterface, const int32_t canProtocol, const int32_t filterMask, const CanId defaultSenderId = 0);
            CanDriver(const string& canInterface, const int32_t canProtocol, const filtermap_t& filters, const CanId defaultSenderId = 0);
            CanDriver() = default;
            virtual ~CanDriver() { uninitialiseSocketCan(); } //!< Destructor

        public: // +++ Getter / Setter +++
            CanDriver&                  setDefaultSenderId(const CanId& id) { this->m_defaultSenderId = id; return *this; } //!< Sets the default sender ID

            CanId                       getDefaultSenderId() const { return this->m_defaultSenderId; } //!< Gets the default sender ID

            filtermap_t                 getFilterMask() const { return this->m_canFilterMask; } //!< Gets the filter mask used by this instance

            int32_t                     getMessageQueueSize() const { return this->m_queueSize; } //!< Gets the amount of CAN messages found after last calling waitForMessages()
            int32_t                     getSocketFd() const { return this->m_socketFd; } //!< The socket file descriptor used by this instance.

            string                      getCanInterface() const { return this->m_canInterface; } //!< The CAN interface used by this instance.

        public: // +++ I/O +++
            #if __cplusplus >= 201300
                #define sockcanpp_3KUS 3000us
                #define sockcanpp_3KMS 3000ms
                #define sockcanpp_3KNS 3000ns

                #define sockcanpp_20US 20us
                #define sockcanpp_20MS 20ms
                #define sockcanpp_20NS 20ns
            #else // C++11
                #define sockcanpp_3KUS std::chrono::microseconds(3000)
                #define sockcanpp_3KMS std::chrono::milliseconds(3000)
                #define sockcanpp_3KNS std::chrono::nanoseconds(3000)

                #define sockcanpp_20US std::chrono::microseconds(20)
                #define sockcanpp_20MS std::chrono::milliseconds(20)
                #define sockcanpp_20NS std::chrono::nanoseconds(20)
            #endif // __cplusplus >= 201300

            virtual bool                waitForMessages(microseconds timeout = sockcanpp_3KUS); //!< Waits for CAN messages to appear
            virtual bool                waitForMessages(milliseconds timeout = sockcanpp_3KMS); //!< Waits for CAN messages to appear
            virtual bool                waitForMessages(nanoseconds timeout = sockcanpp_3KNS); //!< Waits for CAN messages to appear

            virtual CanMessage          readMessage(); //!< Attempts to read a single message from the bus

            virtual ssize_t             sendMessage(const CanMessage& message, bool forceExtended = false); //!< Attempts to send a single CAN message
            virtual ssize_t             sendMessageQueue(queue<CanMessage>& messages, microseconds delay = sockcanpp_20US, bool forceExtended = false); //!< Attempts to send a queue of messages
            virtual ssize_t             sendMessageQueue(queue<CanMessage>& messages, milliseconds delay = sockcanpp_20MS, bool forceExtended = false); //!< Attempts to send a queue of messages
            virtual ssize_t             sendMessageQueue(queue<CanMessage>& messages, nanoseconds delay = sockcanpp_20NS, bool forceExtended = false); //!< Attempts to send a queue of messages

            virtual queue<CanMessage>   readQueuedMessages(); //!< Attempts to read all queued messages from the bus

        public: // +++ Socket Management +++
            virtual void                allowCanFdFrames(const bool enabled = true) const; //!< Sets the CAN FD frame option for the interface
            #ifdef CANXL_XLF
            virtual void                allowCanXlFrames(const bool enabled = true) const; //!< Sets the CAN XL frame option for the interface
            #endif // CANXL_XLF
            virtual void                joinCanFilters() const; //!< Configures the socket to join the CAN filters
            virtual void                setCanFilterMask(const int32_t mask, const CanId& filterId); //!< Attempts to set a new CAN filter mask to the interface
            virtual void                setCanFilters(const filtermap_t& filters); //!< Sets the CAN filters for the interface
            virtual void                setCollectTelemetry(const bool enabled = true); //!< Sets the telemetry collection option for the interface
            virtual void                setErrorFilter(const bool enabled = true) const; //!< Sets the error filter for the interface
            virtual void                setReceiveOwnMessages(const bool enabled = true) const; //!< Sets the receive own messages option for the interface
            virtual void                setReturnRelativeTimestamps(const bool enabled = true) { m_relativeTimestamps = enabled; }

        protected: // +++ Socket Management +++
            virtual void                initialiseSocketCan(); //!< Initialises socketcan
            virtual void                uninitialiseSocketCan(); //!< Uninitialises socketcan

        private: // +++ Member Functions +++
            virtual CanMessage          readMessageLock(bool const lock = true); //!< readMessage deadlock guard
            virtual milliseconds        readFrameTimestamp();

        private: // +++ Variables +++
            bool        m_canReadQueueSize{true}; //!< Is the queue size available
            bool        m_collectTelemetry{false}; //!< Whether or not to collect telemetry data from the CAN bus
            bool        m_relativeTimestamps{false}; //!< Whether or not to use relative timestamps

            CanId       m_defaultSenderId; //!< The ID to send messages with if no other ID was set.

            filtermap_t m_canFilterMask; //!< The bit mask used to filter CAN messages
            
            int32_t     m_canProtocol{CAN_RAW}; //!< The protocol used when communicating via CAN
            int32_t     m_socketFd{-1}; //!< The CAN socket file descriptor
            int32_t     m_queueSize{0}; //!< The size of the message queue read by waitForMessages()

            optional<milliseconds>    m_firstTimestamp{};
            
            mutex       m_lock{}; //!< Mutex for thread-safety.
            mutex       m_lockSend{}; 

            string      m_canInterface; //!< The CAN interface used for communication (e.g. can0, can1, ...)
            
    };

    /**
     * @brief Formats a std string object.
     * 
     * @remarks Yoinked from https://github.com/Beatsleigher/liblogpp :)
     * 
     * @tparam Args The formatting argument types.
     * @param format The format string.
     * @param args The format arguments (strings must be converted to C-style strings!)
     * 
     * @return string The formatted string. 
     */
    template<typename... Args>
    string formatString(const string& format, Args... args)  {
        using std::unique_ptr;
        auto stringSize = snprintf(nullptr, 0, format.c_str(), args...) + 1; // +1 for \0
        unique_ptr<char[]> buffer(new char[stringSize]);

        snprintf(buffer.get(), stringSize, format.c_str(), args...);

        return string(buffer.get(), buffer.get() + stringSize - 1); // std::string handles termination for us.
    }

}

#endif // LIBSOCKCANPP_INCLUDE_CANDRIVER_HPP
