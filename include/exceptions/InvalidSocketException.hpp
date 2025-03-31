/**
 * @file InvalidSocketException.hpp
 * @author Simon Cahill (contact@simonc.eu)
 * @brief Contains the implementation of an exception that may be thrown when an invalid CAN socket is detected.
 * @version 0.1
 * @date 2020-07-02
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

#ifndef LIBSOCKCANPP_INCLUDE_EXCEPTIONS_INVALIDSOCKETEXCEPTION_HPP
#define LIBSOCKCANPP_INCLUDE_EXCEPTIONS_INVALIDSOCKETEXCEPTION_HPP

#include <exception>
#include <string>

namespace sockcanpp { namespace exceptions {

    using std::exception;
    using std::string;

    /**
     * @brief An exception that may be thrown when an error occurs while closing a CAN socket.
     */
    class InvalidSocketException: public exception {
        public: // +++ Constructor / Destructor +++
            InvalidSocketException(const string& message, int32_t socket): m_socket(socket), m_message(message) {}
            ~InvalidSocketException() {}

        public: // +++ Overrides +++
            const char* what() const noexcept { return m_message.c_str(); }

        public: // +++ Getter +++
            const int32_t getSocket() const { return m_socket; }

        private:
            int32_t m_socket;

            string m_message;
    };

} /* exceptions */ } /* sockcanpp */

#endif // LIBSOCKCANPP_INCLUDE_EXCEPTIONS_INVALIDSOCKETEXCEPTION_HPP
