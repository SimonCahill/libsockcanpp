/**
 * @file CanXLMessage.hpp
 * @author Simon Cahill (contact@simonc.eu)
 * @brief Contains the implementation of a CAN message representation in C++.
 * @version 0.1
 * @date 2025-03-25
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

#ifndef LIBSOCKCANPP_INCLUDE_CANXLMESSAGE_HPP
#define LIBSOCKCANPP_INCLUDE_CANXLMESSAGE_HPP

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
    using std::string;
    using std::system_error;

    /**
     * @brief Represents a CAN message that was received.
     */
    class CanMessage {
        public: // +++ Constructor / Destructor +++
            CanMessage(const struct canxl_frame frame): _rawFrame(frame) { }

            CanMessage(const CanId& priorityField, const CanId& acceptanceField, const string& frameData) {
                if (frameData.size() > CANXL_MAX_DLEN) {
                    throw system_error(error_code(0xbadd1c, generic_category()), "Payload too big!");
                }

                struct canxl_frame rawFrame{};

                std::copy(frameData.begin(), frameData.end(), rawFrame.data);

                rawFrame.len = frameData.size();
                rawFrame.prio = static_cast<uint16_t>(priorityField); // Ensure the CanId is truncated to 11 bits
                rawFrame.af = static_cast<uint32_t>(acceptanceField); // Ensure the CanId is truncated to 32 bits
                rawFrame.flags |= CANXL_XLF; // Set the mandatory CAN XL frame flag
                rawFrame.sdt = 0; // Set the SDU type to 0 (default)
            }

            virtual ~CanMessage() = default;

        public: // +++ Getters +++

        private:
            struct canxl_frame _rawFrame;
    };

}

#endif // LIBSOCKCANPP_INCLUDE_CANXLMESSAGE_HPP
