/**
 * @file CanFdMessage.hpp
 * @author Simon Cahill (contact@simonc.eu)
 * @brief Contains the implementation of a CAN FD message representation in C++.
 * @version 0.1
 * @date 2025-01-06
 * 
 * @copyright Copyright (c) 2025 Simon Cahill and Contributors
 */

#ifndef LIBSOCKCANPP_INCLUDE_CANFDMESSAGE_HPP
#define LIBSOCKCANPP_INCLUDE_CANFDMESSAGE_HPP

//////////////////////////////
//      SYSTEM INCLUDES     //
//////////////////////////////
// stl
#include <cstdint>
#include <cstring>
#include <string>

// Check for string_view support
#if __cplusplus >= 201703L
    #include <string_view>
    #define STRING_VIEW_SUPPORTED
#endif

// Check for bit_cast support
#if __cplusplus >= 201703L
    #include <bit>
    #define CUST_REINTR_CAST std::bit_cast
#else
    #define CUST_REINTR_CAST reinterpret_cast
#endif

// libc
#include <linux/can.h>

//////////////////////////////
//      LOCAL  INCLUDES     //
//////////////////////////////
#include "CanId.hpp"
#include "exceptions/CanException.hpp"

namespace sockcanpp::canfd {

    using std::string;

    #ifdef STRING_VIEW_SUPPORTED
        using std::string_view;
    #endif

    /**
     * @brief Represents a CAN FD message that was received.
     */
    class CanFdMessage {
        public: // +++ Static +++
            #ifndef STRING_VIEW_SUPPORTED
            static constexpr bool isMessageValid(const string& frameData)  noexcept { return frameData.size() <= CANFD_MAX_DLEN; }
            #else
            static constexpr bool isMessageValid(const string_view& frameData)  noexcept { return frameData.size() <= CANFD_MAX_DLEN; }
            #endif // !STRING_VIEW_SUPPORTED
            static constexpr bool isMessageValid(const canfd_frame& frame) noexcept { return frame.len <= CANFD_MAX_DLEN;        }

        public: // +++ Constructor / Destructor +++
            CanFdMessage(const struct canfd_frame& frame): m_canIdentifier(frame.can_id), m_frameData({CUST_REINTR_CAST<char*>(frame.data), frame.len}) { }

            CanFdMessage(const CanId canId, const string& frameData): m_canIdentifier(canId), m_frameData(frameData) { }

            #ifdef STRING_VIEW_SUPPORTED
            CanFdMessage(const CanId canId, const string_view& frameData): m_canIdentifier(canId), m_frameData({frameData.begin(), frameData.begin() + frameData.size()}) { }
            #endif // STRING_VIEW_SUPPORTED

            virtual ~CanFdMessage() = default;

        public: // +++ Getters +++
            uint32_t getCanIdentifier() const { return m_canIdentifier; }

            string getFrameData() const { return m_frameData; }

            canfd_frame getRawFrame() const {
                canfd_frame frame{};

                frame.can_id = m_canIdentifier;
                frame.len = static_cast<uint8_t>(m_frameData.size());
                memcpy(frame.data, m_frameData.data(), m_frameData.size());

                return frame;
            }

            public: // +++ Operators +++
                CanFdMessage& operator=(const CanFdMessage& other) {
                    if (this != &other) {
                        m_canIdentifier = other.m_canIdentifier;
                        m_frameData = other.m_frameData;
                    }
    
                    return *this;
                }
    
                CanFdMessage& operator=(CanFdMessage&& other) noexcept {
                    if (this != &other) {
                        m_canIdentifier = other.m_canIdentifier;
                        m_frameData = std::move(other.m_frameData);
                    }
    
                    return *this;
                }
    
                struct canfd_frame operator*() const { return getRawFrame(); }

        public: // +++ Setters +++
            void setCanIdentifier(const uint32_t canId) { m_canIdentifier = canId; }

            #ifdef STRING_VIEW_SUPPORTED
            void setFrameData(const string_view& frameData) {
                if (!isMessageValid(frameData)) { throw exceptions::CanException("Invalid CAN FD frame data length.", -1); }
                m_frameData = {frameData.begin(), frameData.begin() + frameData.size()};
            }
            #else
            void setFrameData(const string& frameData) {
                if (!isMessageValid(frameData)) { throw exceptions::CanException("Invalid CAN FD frame data length.", -1); }
                m_frameData = frameData;
            }
            #endif // !STRING_VIEW_SUPPORTED

        private:
            CanId m_canIdentifier;

            string m_frameData;
    };

}

#endif // LIBSOCKCANPP_INCLUDE_CANFDMESSAGE_HPP