/**
 * @file CanFdDriver.hpp
 * @author Simon Cahill (contact@simonc.eu)
 * @brief Contains the declaration and partial implementation of the CanFD driver class.
 * @version 0.1
 * @date 2025-01-14
 * 
 * @copyright Copyright (c) 2025 Simon Cahill and Contributors.
 */

#ifndef LIBSOCKCANPP_INCLUDE_CANFDDRIVER_HPP
#define LIBSOCKCANPP_INCLUDE_CANFDDRIVER_HPP

//////////////////////////////
//      SYSTEM INCLUDES     //
//////////////////////////////
// stl
#include <chrono>
#include <cstdint>
#include <cstring>
#include <string>
#include <queue>

#if __cplusplus >= 201703L
    #include <string_view>
    #define STRING_VIEW_SUPPORTED
#endif

// libc
#include <linux/can.h>
#include <linux/can/raw.h>

//////////////////////////////
//      LOCAL  INCLUDES     //
//////////////////////////////
#include "CanDriver.hpp"
#include "CanId.hpp"
#include "CanFdMessage.hpp"

namespace sockcanpp::canfd {

    using std::chrono::milliseconds;
    using std::queue;
    using std::string;

    #ifdef STRING_VIEW_SUPPORTED
        using std::string_view;
    #endif

    /**
     * @brief This class wraps around Linux' socketcan FD implementation in C++.
     * This class is designed to be multi-instance capable, and thread-safe.
     * 
     * Its behaviour is similar to that of the @ref CanDriver class.
     */
    class CanFdDriver: public CanDriver {
        public: // +++ Constructor / Destructor +++
            #ifdef STRING_VIEW_SUPPORTED
            #else
            CanFdDriver(const string& canInterface, const int32_t canProtocol, const CanId defaultSenderId): CanDriver(canInterface, canProtocol, defaultSenderId) { }
            CanFdDriver(const string& canInterface, const int32_t canProtocol, const int32_t filterMask, const CanId defaultSenderId = 0);
            CanFdDriver(const string& canInterface, const int32_t canProtocol, const filtermap_t& filters, const CanId defaultSenderId = 0);
            #endif
        protected: // +++ Overrides +++
            void    initialiseSocketCan() override;
    };

}

#endif // LIBSOCKCANPP_INCLUDE_CANFDDRIVER_HPP