/**
 * @file CanXlMessage.hpp
 * @brief Contains the CAN XL message representation.
 * @copyright Copyright (c) 2026 Simon Cahill
 *
 * Licensed under the Apache License, Version 2.0.
 */

#ifndef LIBSOCKCANPP_INCLUDE_CANXLMESSAGE_HPP
#define LIBSOCKCANPP_INCLUDE_CANXLMESSAGE_HPP

#ifndef SOCKCANPP_CANXL_SUPPORT
#error "CanXlMessage.hpp requires sockcanpp_CANXL_SUPPORT=ON"
#endif

#include <linux/can.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iterator>
#if __cpp_lib_span >= 201907L
#include <span>
#endif
#include <string>
#include <system_error>
#include <type_traits>

namespace sockcanpp {

    /**
     * @brief Represents a CAN XL frame.
     *
     * CAN XL uses an 11-bit arbitration priority instead of a classic CAN ID.
     * The mandatory CANXL_XLF flag is added automatically.
     */
    template<typename Duration = std::chrono::milliseconds>
    class CanXlMessageT {
        static_assert(std::is_trivially_copyable<canxl_frame>::value, "canxl_frame must be trivially copyable");

        public:
            CanXlMessageT() { m_rawFrame.flags = CANXL_XLF; }

            explicit CanXlMessageT(const struct canxl_frame& frame): m_rawFrame(frame) {
                m_rawFrame.flags |= CANXL_XLF;
            }

            explicit CanXlMessageT(const struct canxl_frame& frame, const Duration& timestampOffset):
                m_timestampOffset(timestampOffset), m_rawFrame(frame) {
                m_rawFrame.flags |= CANXL_XLF;
            }

            explicit CanXlMessageT(
                const uint16_t priority,
                const std::string& frameData,
                const uint8_t sduType = 0,
                const uint32_t acceptanceField = 0,
                const uint8_t flags = 0,
                const uint8_t virtualCanId = 0
            ) {
                initialise(priority, frameData.begin(), frameData.end(), frameData.size(), sduType, acceptanceField, flags, virtualCanId);
            }

            explicit CanXlMessageT(
                const uint16_t priority,
                const std::string& frameData,
                const Duration& timestampOffset,
                const uint8_t sduType = 0,
                const uint32_t acceptanceField = 0,
                const uint8_t flags = 0,
                const uint8_t virtualCanId = 0
            ): CanXlMessageT(priority, frameData, sduType, acceptanceField, flags, virtualCanId) {
                m_timestampOffset = timestampOffset;
            }

            #if __cpp_lib_span >= 201907L
            explicit CanXlMessageT(
                const uint16_t priority,
                const std::span<const uint8_t>& frameData,
                const uint8_t sduType = 0,
                const uint32_t acceptanceField = 0,
                const uint8_t flags = 0,
                const uint8_t virtualCanId = 0
            ) {
                initialise(priority, frameData.begin(), frameData.end(), frameData.size(), sduType, acceptanceField, flags, virtualCanId);
            }

            explicit CanXlMessageT(
                const uint16_t priority,
                const std::span<const uint8_t>& frameData,
                const Duration& timestampOffset,
                const uint8_t sduType = 0,
                const uint32_t acceptanceField = 0,
                const uint8_t flags = 0,
                const uint8_t virtualCanId = 0
            ): CanXlMessageT(priority, frameData, sduType, acceptanceField, flags, virtualCanId) {
                m_timestampOffset = timestampOffset;
            }
            #endif

            uint16_t getPriority() const noexcept { return static_cast<uint16_t>(m_rawFrame.prio & CANXL_PRIO_MASK); }
            uint8_t getVirtualCanId() const noexcept {
                #ifdef CANXL_VCID_MASK
                return static_cast<uint8_t>((m_rawFrame.prio & CANXL_VCID_MASK) >> CANXL_VCID_OFFSET);
                #else
                return 0;
                #endif
            }
            uint8_t getFlags() const noexcept { return m_rawFrame.flags; }
            uint8_t getSduType() const noexcept { return m_rawFrame.sdt; }
            uint32_t getAcceptanceField() const noexcept { return m_rawFrame.af; }
            size_t getPayloadLength() const noexcept { return m_rawFrame.len; }

            std::string getFrameData() const {
                return std::string(std::begin(m_rawFrame.data), std::begin(m_rawFrame.data) + m_rawFrame.len);
            }

            const canxl_frame& getRawFrame() const noexcept { return m_rawFrame; }
            const Duration& getTimestampOffset() const noexcept { return m_timestampOffset; }

            constexpr bool hasSimpleExtendedContent() const noexcept { return m_rawFrame.flags & CANXL_SEC; }
            constexpr bool hasRemoteRequestSubstitution() const noexcept {
                #ifdef CANXL_RRS
                return m_rawFrame.flags & CANXL_RRS;
                #else
                return false;
                #endif
            }

            bool operator==(const CanXlMessageT& other) const noexcept {
                return m_rawFrame.prio == other.m_rawFrame.prio &&
                       m_rawFrame.flags == other.m_rawFrame.flags &&
                       m_rawFrame.sdt == other.m_rawFrame.sdt &&
                       m_rawFrame.len == other.m_rawFrame.len &&
                       m_rawFrame.af == other.m_rawFrame.af &&
                       std::equal(
                           std::begin(m_rawFrame.data), std::begin(m_rawFrame.data) + m_rawFrame.len,
                           std::begin(other.m_rawFrame.data), std::begin(other.m_rawFrame.data) + other.m_rawFrame.len
                       );
            }

            bool operator!=(const CanXlMessageT& other) const noexcept { return !(*this == other); }

        private:
            template<typename Iterator>
            void initialise(
                const uint16_t priority,
                const Iterator begin,
                const Iterator end,
                const size_t size,
                const uint8_t sduType,
                const uint32_t acceptanceField,
                const uint8_t flags,
                const uint8_t virtualCanId
            ) {
                if (size < CANXL_MIN_DLEN || size > CANXL_MAX_DLEN) {
                    throw std::system_error(std::make_error_code(std::errc::message_size), "CAN XL payload must contain 1 to 2048 bytes!");
                }
                if (priority > CANXL_PRIO_MASK) {
                    throw std::system_error(std::make_error_code(std::errc::invalid_argument), "CAN XL priority must fit in 11 bits!");
                }

                m_rawFrame.prio = priority;
                #ifdef CANXL_VCID_OFFSET
                m_rawFrame.prio |= static_cast<canid_t>(virtualCanId) << CANXL_VCID_OFFSET;
                #else
                if (virtualCanId != 0) {
                    throw std::system_error(std::make_error_code(std::errc::not_supported), "CAN XL VCID is not supported by these kernel headers!");
                }
                #endif
                m_rawFrame.flags = static_cast<uint8_t>(flags | CANXL_XLF);
                m_rawFrame.sdt = sduType;
                m_rawFrame.len = static_cast<uint16_t>(size);
                m_rawFrame.af = acceptanceField;
                std::copy(begin, end, m_rawFrame.data);
            }

            Duration m_timestampOffset{};
            struct canxl_frame m_rawFrame{};
    };

    using CanXlMessage = CanXlMessageT<std::chrono::milliseconds>;

}

#endif // LIBSOCKCANPP_INCLUDE_CANXLMESSAGE_HPP
