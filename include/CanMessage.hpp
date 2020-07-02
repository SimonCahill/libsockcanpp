/**
 * @file CanMessage.hpp
 * @author Simon Cahill (simonc@online.de)
 * @brief Contains the implementation of a CAN message representation in C++.
 * @version 0.1
 * @date 2020-07-01
 * 
 * @copyright Copyright (c) 2020 Simon Cahill
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

            CanMessage(const CanId canId, const string frameData): _canIdentifier(canId), _frameData(frameData) {
                if (frameData.size() > 8) {
                    throw system_error(error_code(0xbad'd1c, generic_category()), "Payload too big!");
                }

                struct can_frame rawFrame;
                rawFrame.can_id = canId;
                memcpy(rawFrame.data, frameData.data(), frameData.size());
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