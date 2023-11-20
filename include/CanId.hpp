/**
 * @file CanId.hpp
 * @author Simon Cahill (simonc@online.de)
 * @brief Contains the implementation of a value-type representing a CAN identifier.
 * @version 0.1
 * @date 2020-07-01
 * 
 * @copyright Copyright (c) 2020 Simon Cahill
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

#ifndef LIBSOCKPP_INCLUDE_CANID_HPP
#define LIBSOCKPP_INCLUDE_CANID_HPP

//////////////////////////////
//      SYSTEM INCLUDES     //
//////////////////////////////
#include <bitset>
#include <cmath>
#include <exception>
#include <linux/can.h>
#include <system_error>

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
            CanId(const CanId& orig): _identifier(orig._identifier), _isErrorFrame(orig._isErrorFrame),
            _isRemoteTransmissionRequest(orig._isRemoteTransmissionRequest), _isStandardFrameId(orig._isStandardFrameId),
            _isExtendedFrameId(orig._isExtendedFrameId) { /* copy */ }

            CanId(const uint32_t identifier): _identifier(identifier) {
                // TODO: Switch to using bitmasks!

                if (isValidIdentifier(identifier)) {
                    if (((int32_t)log2(identifier) + 1) < 11) {
                        _isStandardFrameId = true;
                    } else { _isExtendedFrameId = true; }
                } else if (isErrorFrame(identifier)) {
                    _isErrorFrame = true;
                } else if (isRemoteTransmissionRequest(identifier)) {
                    _isRemoteTransmissionRequest = true;
                }
            }

            CanId(): _identifier(0), _isStandardFrameId(true) { }

        public: // +++ Operators +++

#pragma region "Implicit Cast Operators"
        operator int16_t()  const { return isStandardFrameId() ? (int16_t)_identifier : throw system_error(error_code(0xbad1d, generic_category()), "INVALID CAST: ID is extended or invalid!"); }
        operator uint16_t() const { return isStandardFrameId() ? (uint16_t)_identifier : throw system_error(error_code(0xbad1d, generic_category()), "INVALID CAST: ID is extended or invalid!"); }
        operator int32_t()  const { return _identifier; }
        operator uint32_t() const { return _identifier; }
#pragma endregion

#pragma region "Bitwise Operators"
        CanId    operator &(CanId& x)         const { return _identifier & x._identifier; }
        CanId    operator &(const CanId x)    const { return _identifier & x._identifier; }
        CanId    operator &(const int16_t x)  const { return _identifier & x; }
        CanId    operator &(const uint16_t x) const { return _identifier & x; }
        CanId    operator &(const int32_t x)  const { return _identifier & x; }
        CanId    operator &(const uint32_t x) const { return _identifier & x; }
        CanId    operator &(const int64_t x)  const { return _identifier & x; }
        CanId    operator &(const uint64_t x) const { return _identifier & x; }

        CanId    operator |(CanId& x)         const { return _identifier | x._identifier; }
        CanId    operator |(const CanId x)    const { return _identifier | x._identifier; }
        CanId    operator |(const int16_t x)  const { return _identifier | x; }
        CanId    operator |(const uint16_t x) const { return _identifier | x; }
        CanId    operator |(const int32_t x)  const { return _identifier | x; }
        CanId    operator |(const uint32_t x) const { return _identifier | x; }
        CanId    operator |(const int64_t x)  const { return _identifier | x; }
        CanId    operator |(const uint64_t x) const { return _identifier | x; }
#pragma endregion

#pragma region "Comparison Operators"
        bool operator ==(CanId& x)         const { return _identifier == x._identifier; }
        bool operator ==(const CanId& x)   const { return _identifier == x._identifier; }
        bool operator ==(const int16_t x)  const { return _identifier == x; }
        bool operator ==(const uint16_t x) const { return _identifier == x; }
        bool operator ==(const int32_t x)  const { return _identifier == x; }
        bool operator ==(const uint32_t x) const { return _identifier == x; }
        bool operator ==(const int64_t x)  const { return (x > UINT32_MAX || x < INT32_MIN) ? false : x == _identifier; }
        bool operator ==(const uint64_t x) const { return x > UINT32_MAX ? false : x == _identifier; }
        bool operator !=(CanId& x)         const { return _identifier != x._identifier; }
        bool operator !=(const CanId& x)   const { return _identifier != x._identifier; }
        bool operator !=(const int16_t x)  const { return _identifier != x; }
        bool operator !=(const uint16_t x) const { return _identifier != x; }
        bool operator !=(const int32_t x)  const { return _identifier != x; }
        bool operator !=(const uint32_t x) const { return _identifier != x; }
        bool operator !=(const int64_t x)  const { return (x > UINT32_MAX || x < INT32_MIN) ? false : x != _identifier; }
        bool operator !=(const uint64_t x) const { return x > UINT32_MAX ? false : x != _identifier; }

        bool operator <(CanId& x)          const { return x._identifier < _identifier; }
        bool operator <(int32_t x)         const { return x < _identifier; }
        bool operator <(uint32_t x)        const { return x < _identifier; }
        bool operator <(int16_t x)         const { return x < _identifier; }
        bool operator <(uint16_t x)        const { return x < _identifier; }
        bool operator <=(CanId& x)         const { return x._identifier <= _identifier; }
        bool operator >(CanId& x)          const { return x._identifier > _identifier; }
        bool operator >(int32_t x)         const { return x > _identifier; }
        bool operator >(uint32_t x)        const { return x > _identifier; }
        bool operator >(int16_t x)         const { return x > _identifier; }
        bool operator >(uint16_t x)        const { return x > _identifier; }
        bool operator >=(CanId& x)         const { return x._identifier >= _identifier; }
        bool operator <(const CanId& x)    const { return x._identifier < _identifier; }
        bool operator <=(const CanId& x)   const { return x._identifier <= _identifier; }
        bool operator >(const CanId& x)    const { return x._identifier > _identifier; }
        bool operator >=(const CanId& x)   const { return x._identifier >= _identifier; }
#pragma endregion

#pragma region "Assignment Operators"
        CanId operator =(const int32_t val) {
            uint32_t tmpVal = val;
            auto tmp = (isValidIdentifier(tmpVal) ? CanId(val) : throw system_error(error_code(0x5421, generic_category()), "INVALID CAST: ID is extended or invalid!"));
            return tmp;
        }

        CanId operator =(const uint32_t val) {
            uint32_t tmp = val;
            return (isValidIdentifier(tmp) ? CanId(val) : throw system_error(error_code(0x5421, generic_category()), "INVALID CAST: ID is extended or invalid!"));
        }

        CanId operator =(const int64_t val) { return operator =((int32_t)val); }
#pragma endregion

#pragma region "Arithmetic Operators"
        CanId    operator +(CanId& x)         const { return _identifier + x._identifier; }
        CanId    operator +(const CanId& x)   const { return _identifier + x._identifier; }
        CanId    operator +(const int16_t x)  const { return _identifier + x; }
        CanId    operator +(const uint16_t x) const { return _identifier + x; }
        CanId    operator +(const int32_t x)  const { return _identifier + x; }
        CanId    operator +(const uint32_t x) const { return _identifier + x; }
        CanId    operator +(const int64_t x)  const { return _identifier + x; }
        CanId    operator +(const uint64_t x) const { return _identifier + x; }

        CanId    operator -(CanId& x)         const { return _identifier - x._identifier; }
        CanId    operator -(const CanId& x)   const { return _identifier - x._identifier; }
        CanId    operator -(const int16_t x)  const { return _identifier - x; }
        CanId    operator -(const uint16_t x) const { return _identifier - x; }
        CanId    operator -(const int32_t x)  const { return _identifier - x; }
        CanId    operator -(const uint32_t x) const { return _identifier - x; }
        CanId    operator -(const int64_t x)  const { return _identifier - x; }
        CanId    operator -(const uint64_t x) const { return _identifier - x; }
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
            static bool isValidIdentifier(uint32_t value) {
                int32_t tmpValue = ((int32_t)log2(value) + 2); // Get bit count

                // Check for extended frame flag
                if (tmpValue >= 29) {
                    value = (value & CAN_EFF_FLAG) ? (value & CAN_EFF_MASK) : (value & CAN_SFF_MASK);
                    tmpValue = ((int32_t)log2(value) + 1); // Get bit count again
                }

                return (value == 0) /* Default value, also valid ID */ || ((tmpValue <= 29 && tmpValue > 0));
            }
    
            /**
             * @brief Indicates whether or not a given integer contains the error frame flag or not.
             * 
             * @param value The integer to check.
             * 
             * @return true If value has the error frame flag (bit) set to 1.
             * @return false Otherwise.
             */
            static bool isErrorFrame(uint32_t value) {
                try { return bitset<sizeof(int32_t)>(value).test(29); }
                catch (...) { return false; /* Brute-force, but works. */ }
            }

            /**
             * @brief Indicates whether the received frame is a remote transmission request.
             * 
             * @param value The integer to check.
             * 
             * @return true If the frame is a remote transmission request.
             * @return false Otherwise.
             */
            static bool isRemoteTransmissionRequest(uint32_t value) {
                try { return bitset<sizeof(int32_t)>(value).test(30); }
                catch (...) { return false; /* Brute-force, but works. */ }
            }

        public: // +++ Getters +++
            bool hasErrorFrameFlag() const { return _isErrorFrame; }
            bool hasRtrFrameFlag() const { return _isRemoteTransmissionRequest; }
            bool isStandardFrameId() const { return _isStandardFrameId; }
            bool isExtendedFrameId() const { return _isExtendedFrameId; }

        public: // +++ Equality Checks +++
            bool equals(CanId otherId) const { return *this == otherId; }

        private: // +++ Variables +++
            bool _isErrorFrame = false;
            bool _isRemoteTransmissionRequest = false;
            bool _isStandardFrameId = false;
            bool _isExtendedFrameId = false;

            uint32_t _identifier = 0;
    };

}

#endif // LIBSOCKPP_INCLUDE_CANID_HPP
