/**
 * @file CanControllerError.hpp
 * @author Simon Cahill (contact@simonc.eu)
 * @brief Contains the definition of CAN controller error codes.
 * @version 0.1
 * @date 2025-08-05
 * 
 * @copyright Copyright (c) 2025 Simon Cahill and Contributors.
 */

#ifndef LIBSOCKCANPP_INCLUDE_CAN_ERRORS_CANCONTROLLERERROR_HPP
#define LIBSOCKCANPP_INCLUDE_CAN_ERRORS_CANCONTROLLERERROR_HPP

#include <cstdint>
#include <string>

#include <linux/can/error.h>

#if __cplusplus >= 201703L
namespace sockcanpp::can_errors {
#else
namespace sockcanpp { namespace can_errors {
#endif // __cplusplus >= 201703L

    using std::string;

    /**
     * @brief Contains a typesafe representation of CAN controller error codes.
     */
    enum class ControllerErrorCode: uint8_t {
        UNSPECIFIED_ERROR   = CAN_ERR_CRTL_UNSPEC, //!< Unspecified error.
        RECEIVE_OVERFLOW    = CAN_ERR_CRTL_RX_OVERFLOW, //!< Receive overflow error.
        TRANSMIT_OVERFLOW   = CAN_ERR_CRTL_TX_OVERFLOW, //!< Transmit overflow error.
        RECEIVE_WARNING     = CAN_ERR_CRTL_RX_WARNING, //!< Receive warning error.
        TRANSMIT_WARNING    = CAN_ERR_CRTL_TX_WARNING, //!< Transmit warning error.
        RECEIVE_PASSIVE     = CAN_ERR_CRTL_RX_PASSIVE, //!< Receive passive error.
        TRANSMIT_PASSIVE    = CAN_ERR_CRTL_TX_PASSIVE, //!< Transmit passive error.
        RECOVERED_ACTIVE    = CAN_ERR_CRTL_ACTIVE, //!< Recovered to active state.
    };

    struct ControllerError {
        ControllerErrorCode errorCode;
        string              errorMessage;

        ControllerError(ControllerErrorCode code, const string& message): errorCode(code), errorMessage(message) { }

        static ControllerError fromErrorCode(ControllerErrorCode code) {
            switch (code) {
                case ControllerErrorCode::UNSPECIFIED_ERROR:   return ControllerError(code, "Unspecified error");
                case ControllerErrorCode::RECEIVE_OVERFLOW:    return ControllerError(code, "Receive overflow error");
                case ControllerErrorCode::TRANSMIT_OVERFLOW:   return ControllerError(code, "Transmit overflow error");
                case ControllerErrorCode::RECEIVE_WARNING:     return ControllerError(code, "Receive warning error");
                case ControllerErrorCode::TRANSMIT_WARNING:    return ControllerError(code, "Transmit warning error");
                case ControllerErrorCode::RECEIVE_PASSIVE:     return ControllerError(code, "Receive passive error");
                case ControllerErrorCode::TRANSMIT_PASSIVE:    return ControllerError(code, "Transmit passive error");
                case ControllerErrorCode::RECOVERED_ACTIVE:    return ControllerError(code, "Recovered to active state");
                default:                                       return ControllerError(code, "Unknown error");
            }
        }
    };

#if __cplusplus >= 201703L
} // namespace sockcanpp::can_errors
#else
} /* namespace can_errors */ } /* namespace sockcanpp */
#endif // __cplusplus >= 201703L

namespace std {

    inline string to_string(sockcanpp::can_errors::ControllerErrorCode err) {
        using sockcanpp::can_errors::ControllerErrorCode;
        switch (err) {
            case ControllerErrorCode::UNSPECIFIED_ERROR:   return "Unspecified error";
            case ControllerErrorCode::RECEIVE_OVERFLOW:    return "Receive overflow error";
            case ControllerErrorCode::TRANSMIT_OVERFLOW:   return "Transmit overflow error";
            case ControllerErrorCode::RECEIVE_WARNING:     return "Receive warning error";
            case ControllerErrorCode::TRANSMIT_WARNING:    return "Transmit warning error";
            case ControllerErrorCode::RECEIVE_PASSIVE:     return "Receive passive error";
            case ControllerErrorCode::TRANSMIT_PASSIVE:    return "Transmit passive error";
            case ControllerErrorCode::RECOVERED_ACTIVE:    return "Recovered to active state";
            default:                                       return "Unknown controller error";
        }
    }

}

#endif // LIBSOCKCANPP_INCLUDE_CAN_ERRORS_CANCONTROLLERERROR_HPP