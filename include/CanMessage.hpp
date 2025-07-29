/**
 * @file CanMessage.hpp
 * @author Simon Cahill (contact@simonc.eu)
 * @brief Contains the implementation of a CAN message representation in C++.
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

#ifndef LIBSOCKCANPP_INCLUDE_CANMESSAGE_HPP
#define LIBSOCKCANPP_INCLUDE_CANMESSAGE_HPP

//////////////////////////////
//      SYSTEM INCLUDES     //
//////////////////////////////
#include <linux/can.h>

#include <bit>
#include <cstring>
#include <exception>
#include <string>
#include <system_error>
#include <thread>

//////////////////////////////
//      LOCAL  INCLUDES     //
//////////////////////////////
#include "CanId.hpp"

namespace sockcanpp {

    #if __cpp_lib_bit_cast >= 201806L
    #   define type_cast std::bit_cast
    #else
    #   define type_cast reinterpret_cast
    #endif // __cpp_lib_bit_cast >= 201806L

    using std::chrono::milliseconds;
    using std::error_code;
    using std::generic_category;
    using std::string;
    using std::system_error;

    #if __cpp_lib_string_view >= 201803L
    using std::string_view; //!< Use string_view if available, otherwise use string.
    #endif // __cpp_lib_string_view

    /**
     * @brief Represents a CAN message that was received.
     */
    template<typename Duration = std::chrono::milliseconds>
    class CanMessageT {
        public: // +++ Constructor / Destructor +++
                                CanMessageT() = default; //!< Default constructor, initializes an empty CAN message.

            explicit            CanMessageT(const struct can_frame& frame): m_canIdentifier(frame.can_id), m_rawFrame(frame) { } //!< Constructs a CAN message from a raw can_frame structure.
            explicit            CanMessageT(const struct can_frame& frame, const Duration& timestampOffset): m_canIdentifier(frame.can_id), m_timestampOffset(timestampOffset), m_rawFrame(frame) { } //!< Constructs a CAN message from a raw can_frame structure with a timestamp offset.

            /**
             * @brief Constructs a CAN message from a CAN ID and a frame data string.
             * 
             * @param canId The CAN ID of the message.
             * @param frameData The data of the CAN frame as a string.
             */
            explicit            CanMessageT(const CanId& canId, const string& frameData): m_canIdentifier(canId) {
                if (frameData.size() > CAN_MAX_DLEN) {
                    throw system_error(error_code(std::make_error_code(std::errc::message_size)), "Payload too big!");
                }

                m_rawFrame.can_id = canId;
                std::copy(frameData.begin(), frameData.end(), m_rawFrame.data);
                m_rawFrame.can_dlc = frameData.size();
            }

            explicit            CanMessageT(const CanId& canId, const string& frameData, const Duration& timestampOffset): CanMessageT(canId, frameData) {
                m_timestampOffset = timestampOffset;
            } //!< Constructs a CAN message from a CAN ID and a frame data string with a timestamp offset.

            virtual ~           CanMessageT() = default;

        public: // +++ Getters +++
            const CanId&        getCanId()      const noexcept { return m_canIdentifier; } //!< Returns the CAN ID of this message.

            const string        getFrameData()  const noexcept { return string{type_cast<const char*>(m_rawFrame.data), m_rawFrame.can_dlc}; } //!< Returns the frame data as a string.

            const can_frame&    getRawFrame()   const noexcept { return m_rawFrame; } //!< Returns the raw can_frame structure of this message.

            const Duration&     getTimestampOffset() const noexcept { return m_timestampOffset; } //!< Returns the timestamp offset of this message.

            #if __cpp_lib_string_view >= 201803L
            string_view         getFrameDataView() const noexcept { return string_view(type_cast<const char*>(m_rawFrame.data), m_rawFrame.can_dlc); } //!< Returns a string_view of the frame data for better performance.
            #endif // __cpp_lib_string_view >= 201803L

        private:
            CanId               m_canIdentifier{};

            Duration            m_timestampOffset{};

            struct can_frame    m_rawFrame{};
    };

    class CanMessage : public CanMessageT<> {
        public: // +++ Constructor / Destructor +++
                                CanMessage() = default; //!< Default constructor, initializes an empty CAN message.

            explicit            CanMessage(const struct can_frame& frame): CanMessageT(frame) { } //!< Constructs a CAN message from a raw can_frame structure.
            explicit            CanMessage(const struct can_frame& frame, const milliseconds& timestampOffset): CanMessageT(frame, timestampOffset) { } //!< Constructs a CAN message from a raw can_frame structure with a timestamp offset.

            /**
             * @brief Constructs a CAN message from a CAN ID and a frame data string.
             * 
             * @param canId The CAN ID of the message.
             * @param frameData The data of the CAN frame as a string.
             */
            explicit            CanMessage(const CanId& canId, const string& frameData): CanMessageT(canId, frameData) { }

            explicit            CanMessage(const CanId& canId, const string& frameData, const milliseconds& timestampOffset): CanMessageT(canId, frameData, timestampOffset) { } //!< Constructs a CAN message from a CAN ID and a frame data string with a timestamp offset.

            virtual ~           CanMessage() = default;
    };
}

#endif // LIBSOCKCANPP_INCLUDE_CANMESSAGE_HPP
