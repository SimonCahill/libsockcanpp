/**
 * @file CanInitException.hpp
 * @author Simon Cahill (contact@simonc.eu)
 * @brief Contains the implementation of an exception that is thrown when a CAN socket couldn't be inintialised.
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

#ifndef LIBSOCKCANPP_INCLUDE_EXCEPTIONS_CANINITEXCEPTION_HPP
#define LIBSOCKCANPP_INCLUDE_EXCEPTIONS_CANINITEXCEPTION_HPP

#include <exception>
#include <string>

namespace sockcanpp { namespace exceptions {

    using std::exception;
    using std::string;

    /**
     * @brief An exception that may be thrown when an error occurred while initialising a CAN socket.
     */
    class CanInitException: public exception {
        public: // +++ Constructor / Destructor +++
            CanInitException(string message): _message(message) { }
            virtual ~CanInitException() { }

        public: // +++ Override +++
            const char* what() const noexcept { return _message.c_str(); }

        private:
            string _message;
    };

} /* exceptions */ } /* sockcanpp */

#endif // LIBSOCKCANPP_INCLUDE_EXCEPTIONS_CANINITEXCEPTION_HPP
