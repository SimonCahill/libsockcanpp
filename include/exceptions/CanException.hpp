/**
 * @file CanException.hpp
 * @author Simon Cahill (simonc@online.de)
 * @brief Contains the implementation of a general-purpose exception which may be thrown when an error occurs when performing IO on a CAN socket.
 * @version 0.1
 * @date 2020-07-02
 * 
 * @copyright Copyright (c) 2020 Simon Cahill
 */

#ifndef LIBSOCKCANPP_INCLUDE_EXCEPTIONS_CANEXCEPTION_HPP
#define LIBSOCKCANPP_INCLUDE_EXCEPTIONS_CANEXCEPTION_HPP

#include <exception>
#include <string>

namespace sockcanpp { namespace exceptions {

    using std::exception;
    using std::string;

    /**
     * @brief An exception that may be thrown when an error occurs while closing a CAN socket.
     */
    class CanException: public exception {
        public: // +++ Constructor / Destructor +++
            CanException(string message, int32_t socket): _message(message), _socket(socket) {}
            ~CanException() {}

        public: // +++ Overrides +++
            const char* what() { return _message.c_str(); }

        public: // +++ Getter +++
            const int32_t getSocket() const { return _socket; }

        private:
            int32_t _socket;

            string _message;
    };

} /* exceptions */ } /* sockcanpp */

#endif // LIBSOCKCANPP_INCLUDE_EXCEPTIONS_CANEXCEPTION_HPP