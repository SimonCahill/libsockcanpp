/**
 * @file CanCloseException.hpp
 * @author Simon Cahill (simonc@online.de)
 * @brief Contains the implementation of an exception that may be thrown when an error occurs while closing a CAN socket.
 * @version 0.1
 * @date 2020-07-02
 * 
 * @copyright Copyright (c) 2020 Simon Cahill
 */

#ifndef LIBSOCKCANPP_INCLUDE_EXCEPTIONS_CANCLOSEEXCEPTION_HPP
#define LIBSOCKCANPP_INCLUDE_EXCEPTIONS_CANCLOSEEXCEPTION_HPP

#include <exception>
#include <string>

namespace sockcanpp { namespace exceptions {

    using std::exception;
    using std::string;

    /**
     * @brief An exception that may be thrown when an error occurs while closing a CAN socket.
     */
    class CanCloseException: public exception {
        public: // +++ Constructor / Destructor +++
            CanCloseException(string message): _message(message) {}
            ~CanCloseException() {}

        public: // +++ Overrides +++
            const char* what() { return _message.c_str(); }

        private:
            string _message;
    };

} /* exceptions */ } /* sockcanpp */

#endif // LIBSOCKCANPP_INCLUDE_EXCEPTIONS_CANCLOSEEXCEPTION_HPP