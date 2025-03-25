/**
 * @file CanId.hpp
 * @author Simon Cahill (contact@simonc.eu)
 * @brief Contains the implementation of a value-type representing a CAN identifier.
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

#ifndef LIBSOCKPP_INCLUDE_CANID_HPP
#define LIBSOCKPP_INCLUDE_CANID_HPP

//////////////////////////////
//      SYSTEM INCLUDES     //
//////////////////////////////
#include <algorithm>
#include <bitset>
#include <cmath>
#include <exception>
#include <linux/can.h>
#include <system_error>

#if __cpp_concepts >= 201907
template<typename Str>
concept Stringable = requires(Str s) { { s.data() + s.size() } -> std::convertible_to<const char*>; };

template<typename CharArr>
concept CChar = requires(CharArr c) { std::is_same_v<CharArr, const char*>; };

template<typename Int>
concept Integral = requires(Int i) { std::is_integral_v<Int>; };

template<typename C>
concept ConvertibleToCanId = Stringable<C> || Integral<C> || CChar<C>;
#endif

namespace sockcanpp {

    using std::bitset;
    using std::error_code;
    using std::exception;
    using std::generic_category;
    using std::system_error;

    /**
     * @brief Represents a CAN ID in a simple and easy-to-use manner.
     */
    struct CanId {
        public: // +++ Constructors +++
            constexpr CanId() = default;
            constexpr CanId(const canid_t id): m_identifier(id) { }
            constexpr CanId(const int32_t id): m_identifier(id) { }

            #if __cpp_concepts >= 201907
            template<Stringable T>
            CanId(const T& id) { m_identifier = std::stoul(id.data(), nullptr, 16); }
            #endif // __cpp_concepts >= 201907

            CanId(const char* id) { m_identifier = std::stoul(id, nullptr, 16); }

        public: // +++ Operators +++
            constexpr canid_t operator *() const { return m_identifier; } //!< Returns the raw CAN ID value.

#pragma region "Conversions"
        constexpr operator int16_t()  const { return static_cast<int16_t>(m_identifier) & CAN_ERR_MASK; }
        constexpr operator uint16_t() const { return static_cast<uint16_t>(m_identifier) & CAN_ERR_MASK; }
        constexpr operator int32_t()  const { return m_identifier & CAN_ERR_MASK; }
        constexpr operator canid_t() const { return m_identifier & CAN_ERR_MASK; }
#pragma endregion

#pragma region "Bitwise Operators"
        template<typename T>
        constexpr CanId    operator &(const T x)        const { return m_identifier & x; } //!< Performs a bitwise AND operation on this ID and another.
        constexpr CanId    operator &(const CanId& x)   const { return m_identifier & x.m_identifier; } //!< Performs a bitwise AND operation on this ID and another.

        template<typename T>
        constexpr CanId    operator |(const T x)        const { return m_identifier | x; } //!< Performs a bitwise OR operation on this ID and a 16-bit integer.
        constexpr CanId    operator |(const CanId& x)   const { return m_identifier | x.m_identifier; } //!< Performs a bitwise OR operation on this ID and another.

        template<typename T>
        constexpr CanId    operator ^(const T x)        const { return m_identifier ^ x; } //!< Performs a bitwise XOR operation on this ID and a 16-bit integer.
        constexpr CanId    operator ^(const CanId& x)   const { return m_identifier ^ x.m_identifier; } //!< Performs a bitwise XOR operation on this ID and another.

        constexpr CanId    operator ~()                 const { return ~m_identifier; } //!< Performs a bitwise NOT operation on this ID.

        template<typename T>
        constexpr CanId    operator <<(const T x)       const { return m_identifier << x; } //!< Shifts this ID to the left by a 16-bit integer.
        constexpr CanId    operator <<(const CanId& x)  const { return m_identifier << x.m_identifier; } //!< Shifts this ID to the left by another.

        template<typename T>
        constexpr CanId    operator >>(const T x)       const { return m_identifier >> x; } //!< Shifts this ID to the right by a 16-bit integer.
        constexpr CanId    operator >>(const CanId& x)  const { return m_identifier >> x.m_identifier; } //!< Shifts this ID to the right by another.

        template<typename T>
        CanId    operator <<=(const T x)                      { return m_identifier <<= x; } //!< Shifts this ID to the left by a 16-bit integer.
        CanId    operator <<=(const CanId& x)                 { return m_identifier <<= x.m_identifier; } //!< Shifts this ID to the left by another.

        template<typename T>
        CanId    operator >>=(const T x)                      { return m_identifier >>= x; } //!< Shifts this ID to the right by a 16-bit integer.
        CanId    operator >>=(const CanId& x)                 { return m_identifier >>= x.m_identifier; } //!< Shifts this ID to the right by another.

#pragma endregion

#pragma region "Comparison Operators"
        constexpr bool operator ==(const CanId& x)   const { return m_identifier == x.m_identifier; } //!< Compares this ID to another.

        template<typename T>
        constexpr bool operator ==(const T x)        const { return m_identifier == static_cast<canid_t>(x); } //!< Compares this ID to another.

        constexpr bool operator !=(const CanId& x)   const { return m_identifier != x.m_identifier; } //!< Compares this ID to another.

        template<typename T>
        constexpr bool operator !=(const T x)        const { return m_identifier != static_cast<canid_t>(x); } //!< Compares this ID to another.
        
        template<typename T>
        constexpr bool operator <(T x)               const { return static_cast<canid_t>(x) < m_identifier; } //!< Compares this ID to another.

        template<typename T>
        constexpr bool operator >(T x)               const { return static_cast<canid_t>(x) > m_identifier; } //!< Compares this ID to a 32-bit integer.

        template<typename T>
        constexpr bool operator <=(const T x)        const { return x.m_identifier <= m_identifier; } //!< Compares this ID to another.

        template<typename T>
        constexpr bool operator >=(const T x)        const { return x.m_identifier >= m_identifier; } //!< Compares this ID to another.
#pragma endregion

#pragma region "Assignment Operators"
        template<typename T>
        constexpr CanId operator =(const T val) { return CanId(val); } //!< Assigns a new integer to this CanID

        #if __cpp_concepts >= 201907
        template<Stringable T>
        CanId operator =(const T& val) {
            return operator=(std::stoul(val.data(), nullptr, 16));
        }
        #endif // __cpp_concepts >= 201907

        constexpr CanId operator =(const int64_t val) { return operator =((canid_t)val); } //!< Assigns a 64-bit integer to this ID.

        template<typename T>
        constexpr CanId operator |=(const T x) { return m_identifier |= x; } //!< Performs a bitwise OR operation on this ID and another.

        template<typename T>
        constexpr CanId operator &=(const T x) { return m_identifier &= x; } //!< Performs a bitwise AND operation on this ID and another.

        template<typename T>
        constexpr CanId operator ^=(const T x) { return m_identifier ^= x; } //!< Performs a bitwise XOR operation on this ID and another.
#pragma endregion

#pragma region "Arithmetic Operators"
        template<typename T>
        constexpr CanId    operator +(const T x)    const { return m_identifier + x; }

        template<typename T>
        constexpr CanId    operator +=(const T x)         { return m_identifier += x; }

        template<typename T>
        constexpr CanId    operator -=(const T x)         { return m_identifier -= x; }

        template<typename T>
        constexpr CanId    operator -(const T x)    const { return m_identifier - x; }

        template<typename T>
        constexpr CanId    operator *(const T x)    const { return m_identifier * x; }

        template<typename T>
        constexpr CanId    operator *= (const T x)        { return m_identifier *= x; }

        template<typename T>
        constexpr CanId    operator /(const T x)    const { return m_identifier / x; }

        template<typename T>
        constexpr CanId    operator /=(const T x)         { return m_identifier /= x; }

        template<typename T>
        constexpr CanId    operator %(const T x)    const { return m_identifier % x; }

        template<typename T>
        constexpr CanId    operator %=(const T x)         { return m_identifier %= x; }
#pragma endregion

        public: // +++ Validity Checks +++
            /**
             * @brief Indicates whether or not a given integer is a valid CAN identifier.
             * 
             * @param value The integer to check.
             * 
             * @return true If value is a valid CAN identifier.
             * @return false Otherwise.
             */
            template<typename T>
            static constexpr bool isValidIdentifier(T value) {
                return static_cast<canid_t>(value) <= CAN_EFF_MASK;
            }
    
            /**
             * @brief Indicates whether or not a given integer contains the error frame flag or not.
             * 
             * @param value The integer to check.
             * 
             * @return true If value has the error frame flag (bit) set to 1.
             * @return false Otherwise.
             */
            template<typename T>
            static constexpr bool isErrorFrame(T value) {
                return static_cast<canid_t>(value) & CAN_ERR_FLAG;
            }

            /**
             * @brief Indicates whether the received frame is a remote transmission request.
             * 
             * @param value The integer to check.
             * 
             * @return true If the frame is a remote transmission request.
             * @return false Otherwise.
             */
            template<typename T>
            static constexpr bool isRemoteTransmissionRequest(T value) {
                return static_cast<canid_t>(value) & CAN_RTR_FLAG;
            }

            /**
             * @brief Indicates whether or not a given integer is an extended frame ID.
             * 
             * @param value The integer to check.
             * 
             * @return true If the frame is in the extended format.
             * @return false Otherwise.
             */
            template<typename T>
            static constexpr bool isExtendedFrame(T value) {
                return static_cast<canid_t>(value) & CAN_EFF_FLAG;
            }

        public: // +++ Getters +++
            constexpr bool hasErrorFrameFlag() const { return isErrorFrame(m_identifier); } //!< Indicates whether or not this ID is an error frame.
            constexpr bool hasRtrFrameFlag() const { return isRemoteTransmissionRequest(m_identifier); } //!< Indicates whether or not this ID is a remote transmission request.
            constexpr bool isStandardFrameId() const { return !isExtendedFrame(m_identifier); } //!< Indicates whether or not this ID is a standard frame ID.
            constexpr bool isExtendedFrameId() const { return isExtendedFrame(m_identifier); } //!< Indicates whether or not this ID is an extended frame ID.

        public: // +++ Equality Checks +++
            constexpr bool equals(const CanId& otherId) const { return m_identifier == otherId.m_identifier; } //!< Compares this ID to another.

        private: // +++ Variables +++
            uint32_t m_identifier = 0;
    };

    /**
     * @brief Implements a hash function for the CanId type.
     */
    struct CanIdHasher {
        public:
            size_t operator()(const CanId& id) const { return std::hash<canid_t>()(*id); }
    };

}

#endif // LIBSOCKPP_INCLUDE_CANID_HPP
