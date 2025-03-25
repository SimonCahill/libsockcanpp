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

    using std::error_code;
    using std::generic_category;
    using std::memcpy;
    using std::string;
    using std::system_error;

    /**
     * @brief Represents a CAN message that was received.
     */
    class CanMessage {
        public: // +++ Constructor / Destructor +++
            CanMessage(const struct can_frame frame):
            _canIdentifier(frame.can_id), _frameData((const char*)frame.data, frame.can_dlc), _rawFrame(frame) { }

            CanMessage(const CanId canId, const string& frameData): _canIdentifier(canId), _frameData(frameData) {
                if (frameData.size() > CAN_MAX_DLEN) {
                    throw system_error(error_code(0xbadd1c, generic_category()), "Payload too big!");
                }

                struct can_frame rawFrame{};
                rawFrame.can_id = canId;
                std::copy(frameData.begin(), frameData.end(), rawFrame.data);
                rawFrame.can_dlc = frameData.size();

                _rawFrame = rawFrame;
            }

            virtual ~CanMessage() {}

        public: // +++ Getters +++
            const CanId getCanId() const { return this->_canIdentifier; }
            const string getFrameData() const { return this->_frameData; }
            const can_frame getRawFrame() const { return this->_rawFrame; }

        private:
            CanId _canIdentifier;

            string _frameData;

            struct can_frame _rawFrame;
    };

}

#endif // LIBSOCKCANPP_INCLUDE_CANMESSAGE_HPP
