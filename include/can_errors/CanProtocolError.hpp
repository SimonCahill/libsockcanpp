/**
 * @file CanProtocolError.hpp
 * @author Simon Cahill (contact@simonc.eu)
 * @brief Contains the definition of CAN protocol error codes.
 * @version 0.1
 * @date 2025-08-05
 * 
 * @copyright Copyright (c) 2025 Simon Cahill and Contributors.
 */

#ifndef LIBSOCKCANPP_INCLUDE_CAN_ERRORS_CANPROTOCOLERROR_HPP
#define LIBSOCKCANPP_INCLUDE_CAN_ERRORS_CANPROTOCOLERROR_HPP

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
     * @brief Contains a typesafe representation of CAN protocol error codes.
     */
    enum class ProtocolErrorCode: uint8_t {
        UNSPECIFIED_ERROR   = CAN_ERR_PROT_UNSPEC,  //!< Unspecified error occurred.
        SINGLE_BIT_ERROR    = CAN_ERR_PROT_BIT,     //!< A single bit error occurred.
        FRAME_FORMAT_ERROR  = CAN_ERR_PROT_FORM,    //!< A frame format error occurred.
        BIT_STUFFING_ERROR  = CAN_ERR_PROT_STUFF,   //!< A bit stuffing error occurred.
        DOMINANT_BIT_FAIL   = CAN_ERR_PROT_BIT0,    //!< A dominant bit failure occurred.
        RECESSIVE_BIT_FAIL  = CAN_ERR_PROT_BIT1,    //!< A recessive bit failure occurred.
        OVERLOAD_ERROR      = CAN_ERR_PROT_OVERLOAD,//!< An overload error occurred.
        ACTIVE_ERROR        = CAN_ERR_PROT_ACTIVE,  //!< An active error occurred.
        TX_ERROR            = CAN_ERR_PROT_TX       //!< A transmission error occurred.
    };

    /**
     * @brief Contains a typesafe representation of CAN protocol error locations.
     */
    enum class ProtocolErrorLocation: uint8_t {
        UNSPECIFIED_LOCATION = CAN_ERR_PROT_LOC_UNSPEC,     //!< Unspecified location.
        START_OF_FRAME       = CAN_ERR_PROT_LOC_SOF,        //!< Start of frame.
        IDBIT_28_21          = CAN_ERR_PROT_LOC_ID28_21,    //!< ID bits 28 - 21 (SFF: 10 - 3)
        IDBIT_20_18          = CAN_ERR_PROT_LOC_ID20_18,    //!< ID bits 20 - 18 (SFF: 2 - 0)
        SUBSTITUTE_RTR       = CAN_ERR_PROT_LOC_SRTR,       //!< Substitute RTR bit.
        IDENTIFIER_EXTENSION = CAN_ERR_PROT_LOC_IDE,        //!< Identifier extension bit.
        IDBIT_17_13          = CAN_ERR_PROT_LOC_ID17_13,    //!< ID bits 17 - 13
        IDBIT_12_05          = CAN_ERR_PROT_LOC_ID12_05,    //!< ID bits 12 - 05
        IDBIT_04_00          = CAN_ERR_PROT_LOC_ID04_00,    //!< ID bits 04 - 00
        REMOTE_TRANSMIT_REQ  = CAN_ERR_PROT_LOC_RTR,        //!< Remote transmit request bit.
        RESERVED_BIT_1       = CAN_ERR_PROT_LOC_RES1,       //!< Reserved bit 1.
        RESERVED_BIT_0       = CAN_ERR_PROT_LOC_RES0,       //!< Reserved bit 0.
        DATA_LENGTH_CODE     = CAN_ERR_PROT_LOC_DLC,        //!< Data length code.
        DATA_SECTION         = CAN_ERR_PROT_LOC_DATA,       //!< Data section.
        CRC_SECTION          = CAN_ERR_PROT_LOC_CRC_SEQ,    //!< CRC section.
        CRC_DELIMITER        = CAN_ERR_PROT_LOC_CRC_DEL,    //!< CRC delimiter.
        ACK_SLOT             = CAN_ERR_PROT_LOC_ACK,        //!< ACK slot.
        ACK_DELIMITER        = CAN_ERR_PROT_LOC_ACK_DEL,    //!< ACK delimiter.
        END_OF_FRAME         = CAN_ERR_PROT_LOC_EOF,        //!< End of frame.
        INTERMISSION         = CAN_ERR_PROT_LOC_INTERM,     //!< Intermission section.
    };

    struct ProtocolError {
        ProtocolErrorCode       errorCode;
        ProtocolErrorLocation   errorLocation;
        string                  errorMessage;

        ProtocolError(ProtocolErrorCode code, ProtocolErrorLocation location, const string& message): errorCode(code), errorLocation(location), errorMessage(message) { }

        static ProtocolError fromErrorCode(ProtocolErrorCode code, ProtocolErrorLocation location) {
            switch (code) {
                case ProtocolErrorCode::UNSPECIFIED_ERROR:   return ProtocolError(code, location, "Unspecified error occurred");
                case ProtocolErrorCode::SINGLE_BIT_ERROR:    return ProtocolError(code, location, "Single bit error occurred");
                case ProtocolErrorCode::FRAME_FORMAT_ERROR:  return ProtocolError(code, location, "Frame format error occurred");
                case ProtocolErrorCode::BIT_STUFFING_ERROR:  return ProtocolError(code, location, "Bit stuffing error occurred");
                case ProtocolErrorCode::DOMINANT_BIT_FAIL:   return ProtocolError(code, location, "Dominant bit failure occurred");
                case ProtocolErrorCode::RECESSIVE_BIT_FAIL:  return ProtocolError(code, location, "Recessive bit failure occurred");
                case ProtocolErrorCode::OVERLOAD_ERROR:      return ProtocolError(code, location, "Overload error occurred");
                case ProtocolErrorCode::ACTIVE_ERROR:        return ProtocolError(code, location, "Active error occurred");
                case ProtocolErrorCode::TX_ERROR:            return ProtocolError(code, location, "Transmission error occurred");
                default:                                     return ProtocolError(code, location, "Unknown error occurred");
            }
        }
    };

#if __cplusplus >= 201703L
} // namespace sockcanpp::can_errors
#else
} /* namespace can_errors */ } /* namespace sockcanpp */
#endif // __cplusplus >= 201703L

namespace std {

    inline string to_string(sockcanpp::can_errors::ProtocolErrorLocation loc) {
        using sockcanpp::can_errors::ProtocolErrorLocation;
        switch (loc) {
            case ProtocolErrorLocation::UNSPECIFIED_LOCATION:   return "Unspecified location.";
            case ProtocolErrorLocation::START_OF_FRAME:         return "Start of frame.";
            case ProtocolErrorLocation::IDBIT_28_21:            return "ID bits 28 - 21 (SFF: 10 - 3)";
            case ProtocolErrorLocation::IDBIT_20_18:            return "ID bits 20 - 18 (SFF: 2 - 0)";
            case ProtocolErrorLocation::SUBSTITUTE_RTR:         return "Substitute RTR bit.";
            case ProtocolErrorLocation::IDENTIFIER_EXTENSION:   return "Identifier extension bit.";
            case ProtocolErrorLocation::IDBIT_17_13:            return "ID bits 17 - 13";
            case ProtocolErrorLocation::IDBIT_12_05:            return "ID bits 12 - 05";
            case ProtocolErrorLocation::IDBIT_04_00:            return "ID bits 04 - 00";
            case ProtocolErrorLocation::REMOTE_TRANSMIT_REQ:    return "Remote transmit request bit.";
            case ProtocolErrorLocation::RESERVED_BIT_1:         return "Reserved bit 1.";
            case ProtocolErrorLocation::RESERVED_BIT_0:         return "Reserved bit 0.";
            case ProtocolErrorLocation::DATA_LENGTH_CODE:       return "Data length code.";
            case ProtocolErrorLocation::DATA_SECTION:           return "Data section.";
            case ProtocolErrorLocation::CRC_SECTION:            return "CRC section.";
            case ProtocolErrorLocation::CRC_DELIMITER:          return "CRC delimiter.";
            case ProtocolErrorLocation::ACK_SLOT:               return "ACK slot.";
            case ProtocolErrorLocation::ACK_DELIMITER:          return "ACK delimiter.";
            case ProtocolErrorLocation::END_OF_FRAME:           return "End of frame.";
            case ProtocolErrorLocation::INTERMISSION:           return "Intermission section.";
            default:                                            return "Unknown location.";
        }
    }

    inline string to_string(sockcanpp::can_errors::ProtocolErrorCode err) {
        using sockcanpp::can_errors::ProtocolErrorCode;
        switch (err) {
            case ProtocolErrorCode::UNSPECIFIED_ERROR:   return "Unspecified error occurred";
            case ProtocolErrorCode::SINGLE_BIT_ERROR:    return "Single bit error occurred";
            case ProtocolErrorCode::FRAME_FORMAT_ERROR:  return "Frame format error occurred";
            case ProtocolErrorCode::BIT_STUFFING_ERROR:  return "Bit stuffing error occurred";
            case ProtocolErrorCode::DOMINANT_BIT_FAIL:   return "Dominant bit failure occurred";
            case ProtocolErrorCode::RECESSIVE_BIT_FAIL:  return "Recessive bit failure occurred";
            case ProtocolErrorCode::OVERLOAD_ERROR:      return "Overload error occurred";
            case ProtocolErrorCode::ACTIVE_ERROR:        return "Active error occurred";
            case ProtocolErrorCode::TX_ERROR:            return "Transmission error occurred";
            default:                                     return "Unknown error occurred";
        }
    }

    inline string to_string(sockcanpp::can_errors::ProtocolErrorCode err, sockcanpp::can_errors::ProtocolErrorLocation loc) {
        using sockcanpp::can_errors::ProtocolErrorCode;
        switch (err) {
            case ProtocolErrorCode::UNSPECIFIED_ERROR:   return to_string(err) + " at " + to_string(loc);
            case ProtocolErrorCode::SINGLE_BIT_ERROR:    return to_string(err) + " at " + to_string(loc);
            case ProtocolErrorCode::FRAME_FORMAT_ERROR:  return to_string(err) + " at " + to_string(loc);
            case ProtocolErrorCode::BIT_STUFFING_ERROR:  return to_string(err) + " at " + to_string(loc);
            case ProtocolErrorCode::DOMINANT_BIT_FAIL:   return to_string(err) + " at " + to_string(loc);
            case ProtocolErrorCode::RECESSIVE_BIT_FAIL:  return to_string(err) + " at " + to_string(loc);
            case ProtocolErrorCode::OVERLOAD_ERROR:      return to_string(err) + " at " + to_string(loc);
            case ProtocolErrorCode::ACTIVE_ERROR:        return to_string(err) + " at " + to_string(loc);
            case ProtocolErrorCode::TX_ERROR:            return to_string(err) + " at " + to_string(loc);
            default:                                     return to_string(err) + " at " + to_string(loc);
        }
    }

}

#endif // LIBSOCKCANPP_INCLUDE_CAN_ERRORS_CANPROTOCOLERROR_HPP