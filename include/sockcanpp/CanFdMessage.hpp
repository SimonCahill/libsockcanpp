/**
 * @file CanFdMessage.hpp
 * @author Simon Cahill (contact@simonc.eu)
 * @brief Contains the implementation of a CAN FD message representation in C++.
 * @version 0.1
 * @date 2026-06-16
 *
 * @copyright Copyright (c) 2026 Simon Cahill
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

#ifndef LIBSOCKCANPP_INCLUDE_CANFDMESSAGE_HPP
#define LIBSOCKCANPP_INCLUDE_CANFDMESSAGE_HPP

//////////////////////////////
//      SYSTEM INCLUDES     //
//////////////////////////////
#include <linux/can.h>

#include <algorithm>
#include <chrono>
#include <cstring>
#if __cpp_lib_span >= 201907L
#include <span>
#endif // __cpp_lib_span >= 201907L
#include <iterator>
#include <string>
#include <system_error>
#include <type_traits>

//////////////////////////////
//      LOCAL  INCLUDES     //
//////////////////////////////
#include "CanId.hpp"

namespace sockcanpp {

    using std::string;
    using std::system_error;

    #if __cplusplus >= 201703L
    #define NO_DISCARD [[nodiscard]]
    #else
    #define NO_DISCARD
    #endif // __cplusplus >= 201703L

    /**
     * @brief Represents a CAN FD message that was received.
     */
    template<typename Duration = std::chrono::milliseconds>
    class CanFdMessageT {
        static_assert(std::is_trivially_copyable<canfd_frame>::value, "canfd_frame must be trivially copyable");

        public: // +++ Constructor / Destructor +++
                                CanFdMessageT() = default; //!< Default constructor, initializes an empty CAN FD message.

            explicit            CanFdMessageT(const struct canfd_frame& frame): m_canIdentifier(frame.can_id), m_rawFrame(frame) { } //!< Constructs a CAN FD message from a raw canfd_frame structure.
            explicit            CanFdMessageT(const struct canfd_frame& frame, const Duration& timestampOffset): m_canIdentifier(frame.can_id), m_timestampOffset(timestampOffset), m_rawFrame(frame) { } //!< Constructs a CAN FD message from a raw canfd_frame structure with a timestamp offset.

            explicit            CanFdMessageT(const struct can_frame& frame): m_canIdentifier(frame.can_id) {
                m_rawFrame.can_id = frame.can_id;
                m_rawFrame.len = frame.can_dlc;
                std::copy(std::begin(frame.data), std::begin(frame.data) + frame.can_dlc, m_rawFrame.data);
            } //!< Constructs a CAN FD message from a raw classic can_frame structure.

            explicit            CanFdMessageT(const struct can_frame& frame, const Duration& timestampOffset): CanFdMessageT(frame) {
                m_timestampOffset = timestampOffset;
            } //!< Constructs a CAN FD message from a raw classic can_frame structure with a timestamp offset.

            explicit            CanFdMessageT(const CanId& canId, const string& frameData, const uint8_t flags = 0): m_canIdentifier(canId) {
                if (frameData.size() > CANFD_MAX_DLEN) {
                    throw system_error(std::make_error_code(std::errc::message_size), "CAN FD payload too big!");
                }

                m_rawFrame.can_id = canId;
                m_rawFrame.len = frameData.size();
                m_rawFrame.flags = flags;
                std::copy(frameData.begin(), frameData.end(), m_rawFrame.data);
            } //!< Constructs a CAN FD message from a CAN ID and a frame data string.

            explicit            CanFdMessageT(const CanId& canId, const string& frameData, const Duration& timestampOffset, const uint8_t flags = 0): CanFdMessageT(canId, frameData, flags) {
                m_timestampOffset = timestampOffset;
            } //!< Constructs a CAN FD message from a CAN ID and a frame data string with a timestamp offset.

            #if __cpp_lib_span >= 201907L
            explicit            CanFdMessageT(const CanId& canId, const std::span<const uint8_t>& frameData, const uint8_t flags = 0): m_canIdentifier(canId) {
                if (frameData.size() > CANFD_MAX_DLEN) {
                    throw system_error(std::make_error_code(std::errc::message_size), "CAN FD payload too big!");
                }

                m_rawFrame.can_id = canId;
                m_rawFrame.len = frameData.size();
                m_rawFrame.flags = flags;
                std::copy(frameData.begin(), frameData.end(), m_rawFrame.data);
            } //!< Constructs a CAN FD message from a CAN ID and a span of bytes.

            explicit            CanFdMessageT(const CanId& canId, const std::span<const uint8_t>& frameData, const Duration& timestampOffset, const uint8_t flags = 0): CanFdMessageT(canId, frameData, flags) {
                m_timestampOffset = timestampOffset;
            } //!< Constructs a CAN FD message from a CAN ID and a span of bytes with a timestamp offset.
            #endif // __cpp_lib_span >= 201907L

                                CanFdMessageT(const CanFdMessageT& other) = default; //!< Copy constructor.
                                CanFdMessageT(CanFdMessageT&& other) noexcept = default; //!< Move constructor.
                                CanFdMessageT& operator=(const CanFdMessageT& other) = default; //!< Copy assignment operator.
                                CanFdMessageT& operator=(CanFdMessageT&& other) noexcept = default; //!< Move assignment operator.

            virtual ~           CanFdMessageT() = default;

        public: // +++ Public API +++
            const CanId&        getCanId()      const noexcept { return m_canIdentifier; } //!< Returns the CAN ID of this message.

            string              getFrameData()  const noexcept {
                string data{};
                data.reserve(m_rawFrame.len);
                std::copy(std::begin(m_rawFrame.data), std::begin(m_rawFrame.data) + m_rawFrame.len, std::back_inserter(data));

                return data;
            } //!< Returns the frame data as a string.

            const canfd_frame&  getRawFrame()   const noexcept { return m_rawFrame; } //!< Returns the raw canfd_frame structure of this message.

            const Duration&     getTimestampOffset() const noexcept { return m_timestampOffset; } //!< Returns the timestamp offset of this message.

            uint8_t             getFlags() const noexcept { return m_rawFrame.flags; } //!< Returns the CAN FD frame flags.
            size_t              getPayloadLength() const noexcept { return m_rawFrame.len; } //!< Returns the CAN FD payload length.

            NO_DISCARD constexpr bool hasBitRateSwitch() const noexcept { return m_rawFrame.flags & CANFD_BRS; } //!< Checks if bit-rate switching is enabled.
            NO_DISCARD constexpr bool hasErrorStateIndicator() const noexcept { return m_rawFrame.flags & CANFD_ESI; } //!< Checks if the error state indicator is set.
            NO_DISCARD constexpr bool isErrorFrame() const noexcept { return m_canIdentifier.hasErrorFrameFlag(); } //!< Checks if the CAN FD message is an error frame.
            NO_DISCARD constexpr bool isStandardFrameId() const noexcept { return m_canIdentifier.isStandardFrameId(); } //!< Checks if the CAN FD message has a standard frame ID.
            NO_DISCARD constexpr bool isExtendedFrameId() const noexcept { return m_canIdentifier.isExtendedFrameId(); } //!< Checks if the CAN FD message has an extended frame ID.

        public: // +++ Equality Checks +++
            bool                operator==(const CanFdMessageT& other) const noexcept {
                return  m_canIdentifier == other.m_canIdentifier &&
                        m_rawFrame.len == other.m_rawFrame.len &&
                        m_rawFrame.flags == other.m_rawFrame.flags &&
                        std::equal(
                            std::begin(m_rawFrame.data), std::begin(m_rawFrame.data) + m_rawFrame.len,
                            std::begin(other.m_rawFrame.data), std::begin(other.m_rawFrame.data) + other.m_rawFrame.len
                        );
            } //!< Compares this CAN FD message to another for equality.

            bool                operator!=(const CanFdMessageT& other) const noexcept {
                return !(*this == other);
            } //!< Compares this CAN FD message to another for inequality.

        private:
            CanId               m_canIdentifier{};

            Duration            m_timestampOffset{};

            struct canfd_frame  m_rawFrame{};
    };

    using CanFdMessage = CanFdMessageT<std::chrono::milliseconds>; //!< Default CAN FD message type with milliseconds as the timestamp offset.

}

#endif // LIBSOCKCANPP_INCLUDE_CANFDMESSAGE_HPP
