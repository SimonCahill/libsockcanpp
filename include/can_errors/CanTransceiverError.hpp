/**
 * @file CanTransceiverError.hpp
 * @author Simon Cahill (contact@simonc.eu)
 * @brief Contains the definition of CAN transceiver error codes.
 * @version 0.1
 * @date 2025-08-05
 * 
 * @copyright Copyright (c) 2025 Simon Cahill and Contributors.
 */

#ifndef LIBSOCKCANPP_INCLUDE_CAN_ERRORS_CANTRANSCEIVERERROR_HPP
#define LIBSOCKCANPP_INCLUDE_CAN_ERRORS_CANTRANSCEIVERERROR_HPP

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
     * @brief Contains a typesafe representation of CAN transceiver error codes.
     */
    enum class TransceiverErrorCode: uint8_t {
                                                                    //                                  CANH CANL
        UNSPECIFIED_ERROR       =   CAN_ERR_TRX_UNSPEC,             //!< Unspecified error.             0000 0000
        CAN_HIGH_NO_WIRE        =   CAN_ERR_TRX_CANH_NO_WIRE,       //!< CANH no wire error.            0000 0100
        CAN_HIGH_SHORT_TO_BAT   =   CAN_ERR_TRX_CANH_SHORT_TO_BAT,  //!< CANH short to battery error.   0000 0101
        CAN_HIGH_SHORT_TO_VCC   =   CAN_ERR_TRX_CANH_SHORT_TO_VCC,  //!< CANH short to VCC error.       0000 0110
        CAN_HIGH_SHORT_TO_GND   =   CAN_ERR_TRX_CANH_SHORT_TO_GND,  //!< CANH short to ground error.    0000 0111

        CAN_LOW_NO_WIRE         =   CAN_ERR_TRX_CANL_NO_WIRE,       //!< CANL no wire error.            0100 0000
        CAN_LOW_SHORT_TO_BAT    =   CAN_ERR_TRX_CANL_SHORT_TO_BAT,  //!< CANL short to battery error.   0101 0000
        CAN_LOW_SHORT_TO_VCC    =   CAN_ERR_TRX_CANL_SHORT_TO_VCC,  //!< CANL short to VCC error.       0110 0000
        CAN_LOW_SHORT_TO_GND    =   CAN_ERR_TRX_CANL_SHORT_TO_GND,  //!< CANL short to ground error.    0111 0000
        CAN_LOW_SHORT_TO_HIGH   =   CAN_ERR_TRX_CANL_SHORT_TO_CANH, //!< CANL short to CANH error.      1000 0000
    };

    struct TransceiverError {
        TransceiverErrorCode errorCode;
        string              errorMessage;

        TransceiverError(TransceiverErrorCode code, const string& message): errorCode(code), errorMessage(message) { }

        static TransceiverError fromErrorCode(TransceiverErrorCode code) {
            switch (code) {
                case TransceiverErrorCode::UNSPECIFIED_ERROR:       return TransceiverError(code, "Unspecified error.");
                case TransceiverErrorCode::CAN_HIGH_NO_WIRE:        return TransceiverError(code, "CANH no wire error.");
                case TransceiverErrorCode::CAN_HIGH_SHORT_TO_BAT:   return TransceiverError(code, "CANH short to battery error.");
                case TransceiverErrorCode::CAN_HIGH_SHORT_TO_VCC:   return TransceiverError(code, "CANH short to VCC error.");
                case TransceiverErrorCode::CAN_HIGH_SHORT_TO_GND:   return TransceiverError(code, "CANH short to ground error.");
                case TransceiverErrorCode::CAN_LOW_NO_WIRE:         return TransceiverError(code, "CANL no wire error.");
                case TransceiverErrorCode::CAN_LOW_SHORT_TO_BAT:    return TransceiverError(code, "CANL short to battery error.");
                case TransceiverErrorCode::CAN_LOW_SHORT_TO_VCC:    return TransceiverError(code, "CANL short to VCC error.");
                case TransceiverErrorCode::CAN_LOW_SHORT_TO_GND:    return TransceiverError(code, "CANL short to ground error.");
                case TransceiverErrorCode::CAN_LOW_SHORT_TO_HIGH:   return TransceiverError(code, "CANL short to CANH error.");
                default:                                            return TransceiverError(code, "Unknown error.");
            }
        }
    };

#if __cplusplus >= 201703L
} // namespace sockcanpp::can_errors
#else
} /* namespace can_errors */ } /* namespace sockcanpp */
#endif // __cplusplus >= 201703L

namespace std {

    inline string to_string(sockcanpp::can_errors::TransceiverErrorCode err) {
        using sockcanpp::can_errors::TransceiverErrorCode;
        switch (err) {
            case TransceiverErrorCode::UNSPECIFIED_ERROR:       return "Unspecified error.";
            case TransceiverErrorCode::CAN_HIGH_NO_WIRE:        return "CANH no wire error.";
            case TransceiverErrorCode::CAN_HIGH_SHORT_TO_BAT:   return "CANH short to battery error.";
            case TransceiverErrorCode::CAN_HIGH_SHORT_TO_VCC:   return "CANH short to VCC error.";
            case TransceiverErrorCode::CAN_HIGH_SHORT_TO_GND:   return "CANH short to ground error.";
            case TransceiverErrorCode::CAN_LOW_NO_WIRE:         return "CANL no wire error.";
            case TransceiverErrorCode::CAN_LOW_SHORT_TO_BAT:    return "CANL short to battery error.";
            case TransceiverErrorCode::CAN_LOW_SHORT_TO_VCC:    return "CANL short to VCC error.";
            case TransceiverErrorCode::CAN_LOW_SHORT_TO_GND:    return "CANL short to ground error.";
            case TransceiverErrorCode::CAN_LOW_SHORT_TO_HIGH:   return "CANL short to CANH error.";
            default:                                            return "Unknown error.";
        }
    }

}

#endif // LIBSOCKCANPP_INCLUDE_CAN_ERRORS_CANTRANSCEIVERERROR_HPP