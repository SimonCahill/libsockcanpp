/**
 * @file CanInitException.hpp
 * @author Simon Cahill (simonc@online.de)
 * @brief Contains the implementation of an exception that is thrown when a CAN socket couldn't be inintialised.
 * @version 0.1
 * @date 2020-07-02
 * 
 * @copyright Copyright (c) 2020 Simon Cahill
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
            const char* what() { return _message.c_str(); }

        private:
            string _message;
    };

} /* exceptions */ } /* sockcanpp */

#endif // LIBSOCKCANPP_INCLUDE_EXCEPTIONS_CANINITEXCEPTION_HPP