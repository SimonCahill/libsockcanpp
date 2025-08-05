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
#if __cpp_lib_span >= 201907L
#include <span>
#endif // __cpp_lib_span >= 201907L
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
        static_assert(std::is_trivially_copyable<can_frame>::value, "can_frame must be trivially copyable"); //!< Ensures that the can_frame structure is trivially copyable, which is necessary for efficient copying and moving of CAN messages.

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
                    throw system_error(std::make_error_code(std::errc::message_size), "Payload too big!");
                }

                m_rawFrame.can_id = canId;
                std::copy(frameData.begin(), frameData.end(), m_rawFrame.data);
                m_rawFrame.can_dlc = frameData.size();
            }

            explicit            CanMessageT(const CanId& canId, const string& frameData, const Duration& timestampOffset): CanMessageT(canId, frameData) {
                m_timestampOffset = timestampOffset;
            } //!< Constructs a CAN message from a CAN ID and a frame data string with a timestamp offset.

            #if __cpp_lib_span >= 201907L
            /**
             * @brief Constructs a CAN message from a CAN ID and a span of bytes.
             * 
             * @param canId The CAN ID of the message.
             * @param frameData The data of the CAN frame as a span of bytes.
             */
            explicit            CanMessageT(const CanId& canId, const std::span<const uint8_t>& frameData): m_canIdentifier(canId) {
                if (frameData.size() > CAN_MAX_DLEN) {
                    throw system_error(std::make_error_code(std::errc::message_size), "Payload too big!");
                }

                m_rawFrame.can_id = canId;
                std::copy(frameData.begin(), frameData.end(), m_rawFrame.data);
                m_rawFrame.len = frameData.size();
            }

            explicit            CanMessageT(const CanId& canId, const std::span<const uint8_t>& frameData, const Duration& timestampOffset): CanMessageT(canId, frameData) {
                m_timestampOffset = timestampOffset;
            } //!< Constructs a CAN message from a CAN ID and a span of bytes with a timestamp offset.
            #endif // __cpp_lib_span >= 201907L

                                CanMessageT(const CanMessageT& other) = default; //!< Copy constructor.
                                CanMessageT(CanMessageT&& other) noexcept = default; //!< Move constructor.
                                CanMessageT& operator=(const CanMessageT& other) = default; //!< Copy assignment operator.
                                CanMessageT& operator=(CanMessageT&& other) noexcept = default; //!< Move assignment operator.

            virtual ~           CanMessageT() = default;

        public: // +++ Public API +++
            const CanId&        getCanId()      const noexcept { return m_canIdentifier; } //!< Returns the CAN ID of this message.

            const string        getFrameData()  const noexcept {
                string data{};
                data.reserve(m_rawFrame.len);
                std::copy(std::begin(m_rawFrame.data), std::begin(m_rawFrame.data) + m_rawFrame.can_dlc, std::back_inserter(data));

                return data;
            } //!< Returns the frame data as a string.

            const can_frame&    getRawFrame()   const noexcept { return m_rawFrame; } //!< Returns the raw can_frame structure of this message.

            const Duration&     getTimestampOffset() const noexcept { return m_timestampOffset; } //!< Returns the timestamp offset of this message.

            [[nodiscard]] constexpr bool isErrorFrame() const noexcept { return m_canIdentifier.hasErrorFrameFlag(); } //!< Checks if the CAN message is an error frame.

            [[nodiscard]] constexpr bool isRemoteTransmissionRequest() const noexcept { return m_canIdentifier.hasRtrFrameFlag(); } //!< Checks if the CAN message is a remote transmission request.

            [[nodiscard]] constexpr bool isStandardFrameId() const noexcept { return m_canIdentifier.isStandardFrameId(); } //!< Checks if the CAN message has a standard frame ID.

            [[nodiscard]] constexpr bool isExtendedFrameId() const noexcept { return m_canIdentifier.isExtendedFrameId(); } //!< Checks if the CAN message has an extended frame ID.

        public: // +++ Equality Checks +++
            bool                operator==(const CanMessageT& other) const noexcept {
                return  m_canIdentifier == other.m_canIdentifier &&
                        m_rawFrame.can_dlc == other.m_rawFrame.can_dlc &&
                        std::equal(
                            std::begin(m_rawFrame.data), std::begin(m_rawFrame.data) + m_rawFrame.can_dlc,
                            std::begin(other.m_rawFrame.data), std::begin(other.m_rawFrame.data) + other.m_rawFrame.can_dlc
                        );
            } //!< Compares this CAN message to another for equality.

            bool                operator!=(const CanMessageT& other) const noexcept {
                return !(*this == other);
            } //!< Compares this CAN message to another for inequality.

        private:
            CanId               m_canIdentifier{};

            Duration            m_timestampOffset{};

            struct can_frame    m_rawFrame{};
    };

    using CanMessage = CanMessageT<std::chrono::milliseconds>; //!< Default CAN message type with milliseconds as the timestamp offset.

}

#endif // LIBSOCKCANPP_INCLUDE_CANMESSAGE_HPP
