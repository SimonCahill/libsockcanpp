/**
 * @file CanMessage_ErrorFrameTests.cpp
 * @author Simon Cahill (contact@simonc.eu)
 * @brief Contains all the unit tests for the error frame handling in the CanMessage structure.
 * @version 0.1
 * @date 2025-08-05
 * 
 * @copyright Copyright (c) 2025 Simon Cahill and Contributors.
 */

#include <gtest/gtest.h>

#include <CanMessage.hpp>

#include <array>

using sockcanpp::CanId;
using sockcanpp::CanMessage;
using namespace sockcanpp::can_errors;

using std::array;
using std::string;

TEST(CanMessageErrorFrameTests, CanMessage_ErrorFrame_ExpectTxTimeout) {
    CanMessage msg(CanId(CAN_ERR_FLAG | CAN_ERR_TX_TIMEOUT), "");

    ASSERT_TRUE(msg.isTxTimeout());
}

TEST(CanMessageErrorFrameTests, CanMessage_ErrorFrame_ExpectLostArbitration_Bit1) {
    CanMessage msg(CanId(CAN_ERR_FLAG | CAN_ERR_LOSTARB), "\x01");

    ASSERT_TRUE(msg.hasLostArbitration());
    ASSERT_EQ(msg.arbitrationLostInBit(), 1);
}

TEST(CanMessageErrorFrameTests, CanMessage_ErrorFrame_ExpectLostArbitration_Bit10) {
    CanMessage msg(CanId(CAN_ERR_FLAG | CAN_ERR_LOSTARB), "\x0a");

    ASSERT_TRUE(msg.hasLostArbitration());
    ASSERT_EQ(msg.arbitrationLostInBit(), 10);
}

TEST(CanMessageErrorFrameTests, CanMessage_ErrorFrame_ExpectLostArbitration_Bit100) {
    CanMessage msg(CanId(CAN_ERR_FLAG | CAN_ERR_LOSTARB), "\x64");

    ASSERT_TRUE(msg.hasLostArbitration());
    ASSERT_EQ(msg.arbitrationLostInBit(), 100);
}

TEST(CanMessageErrorFrameTests, CanMessage_ErrorFrame_ExpectLostArbitration_TestAllBits) {
    for (uint8_t i = 0; i < 0xff; i++) {
        CanMessage msg(CanId(CAN_ERR_FLAG | CAN_ERR_LOSTARB), string(1, i));

        ASSERT_TRUE(msg.hasLostArbitration());
        ASSERT_EQ(msg.arbitrationLostInBit(), i);
    }
}

TEST(CanMessageErrorFrameTests, CanMessage_ErrorFrame_ControllerProblem_ExpectUnspecified) {
    CanMessage msg(CanId(CAN_ERR_FLAG | CAN_ERR_CRTL), "\xff\x00");

    ASSERT_TRUE(msg.hasControllerProblem());
    ASSERT_EQ(msg.getControllerError().errorCode, ControllerErrorCode::UNSPECIFIED_ERROR);
}

TEST(CanMessageErrorFrameTests, CanMessage_ErrorFrame_ControllerProblem_ExpectRxOverflow) {
    CanMessage msg(CanId(CAN_ERR_FLAG | CAN_ERR_CRTL), "\xff\x01");

    ASSERT_TRUE(msg.hasControllerProblem());
    ASSERT_EQ(msg.getControllerError().errorCode, ControllerErrorCode::RECEIVE_OVERFLOW);
}

TEST(CanMessageErrorFrameTests, CanMessage_ErrorFrame_ControllerProblem_ExpectTxOverflow) {
    CanMessage msg(CanId(CAN_ERR_FLAG | CAN_ERR_CRTL), "\xff\x02");

    ASSERT_TRUE(msg.hasControllerProblem());
    ASSERT_EQ(msg.getControllerError().errorCode, ControllerErrorCode::TRANSMIT_OVERFLOW);
}

TEST(CanMessageErrorFrameTests, CanMessage_ErrorFrame_ControllerProblem_ExpectRxWarning) {
    CanMessage msg(CanId(CAN_ERR_FLAG | CAN_ERR_CRTL), "\xff\x04");

    ASSERT_TRUE(msg.hasControllerProblem());
    ASSERT_EQ(msg.getControllerError().errorCode, ControllerErrorCode::RECEIVE_WARNING);
}

TEST(CanMessageErrorFrameTests, CanMessage_ErrorFrame_ControllerProblem_ExpectTxWarning) {
    CanMessage msg(CanId(CAN_ERR_FLAG | CAN_ERR_CRTL), "\xff\x08");

    ASSERT_TRUE(msg.hasControllerProblem());
    ASSERT_EQ(msg.getControllerError().errorCode, ControllerErrorCode::TRANSMIT_WARNING);
}

TEST(CanMessageErrorFrameTests, CanMessage_ErrorFrame_ControllerProblem_ExpectRxPassive) {
    CanMessage msg(CanId(CAN_ERR_FLAG | CAN_ERR_CRTL), "\xff\x10");

    ASSERT_TRUE(msg.hasControllerProblem());
    ASSERT_EQ(msg.getControllerError().errorCode, ControllerErrorCode::RECEIVE_PASSIVE);
}

TEST(CanMessageErrorFrameTests, CanMessage_ErrorFrame_ControllerProblem_ExpectTxPassive) {
    CanMessage msg(CanId(CAN_ERR_FLAG | CAN_ERR_CRTL), "\xff\x20");

    ASSERT_TRUE(msg.hasControllerProblem());
    ASSERT_EQ(msg.getControllerError().errorCode, ControllerErrorCode::TRANSMIT_PASSIVE);
}

TEST(CanMessageErrorFrameTests, CanMessage_ErrorFrame_ControllerProblem_ExpectRecoveredActive) {
    CanMessage msg(CanId(CAN_ERR_FLAG | CAN_ERR_CRTL), "\xff\x40");

    ASSERT_TRUE(msg.hasControllerProblem());
    ASSERT_EQ(msg.getControllerError().errorCode, ControllerErrorCode::RECOVERED_ACTIVE);
}

TEST(CanMessageErrorFrameTests, CanMessage_ErrorFrame_ProtocolError_ExpectCombinations) {
    array<uint8_t, 9> protType{ 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
    array<uint8_t, 20> protLocation{ 0x00, 0x03, 0x02, 0x06, 0x04, 0x05, 0x07, 0x0F, 0x0E, 0x0C, 0x0D, 0x09, 0x0B, 0x0A, 0x08, 0x18, 0x19, 0x1B, 0x1A, 0x12 };

    for (const auto i : protType) {
        for (const auto j : protLocation) {
            CanMessage msg(CanId(CAN_ERR_FLAG | CAN_ERR_PROT), string({ static_cast<char>(0xff), static_cast<char>(0xff), static_cast<char>(i), static_cast<char>(j) }));

            ASSERT_TRUE(msg.hasProtocolViolation());
            ASSERT_EQ(msg.getProtocolError().errorCode, static_cast<ProtocolErrorCode>(i));
            ASSERT_EQ(msg.getProtocolError().errorLocation, static_cast<ProtocolErrorLocation>(j));
        }
    }
}
