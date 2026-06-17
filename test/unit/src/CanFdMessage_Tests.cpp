/**
 * @file CanFdMessage_Tests.cpp
 * @author Simon Cahill (contact@simonc.eu)
 * @brief Contains all the unit tests for the CanFdMessage structure.
 * @version 0.1
 * @date 2026-06-16
 *
 * @copyright Copyright (c) 2026 Simon Cahill and Contributors.
 */

#include <gtest/gtest.h>

#include <CanFdMessage.hpp>
#include <CanDriver.hpp>

#include <linux/can.h>

#include <string>
#include <system_error>

using sockcanpp::CanDriver;
using sockcanpp::CanFdMessage;

using std::string;

TEST(CanFdMessageTests, CanFdMessage_Constructor_ExpectDefaultValues) {
    CanFdMessage msg;

    ASSERT_EQ(msg.getCanId(), 0);
    ASSERT_EQ(msg.getFrameData(), "");
    ASSERT_EQ(msg.getPayloadLength(), 0);
    ASSERT_EQ(msg.getFlags(), 0);
    ASSERT_TRUE(msg.getRawFrame().can_id == 0 && msg.getRawFrame().len == 0);
}

TEST(CanFdMessageTests, CanFdMessage_ConstructorWithId_ExpectCorrectIdAndFdPayload) {
    const string payload(CANFD_MAX_DLEN, 'a');
    CanFdMessage msg(0x123, payload);

    ASSERT_EQ(msg.getCanId(), 0x123);
    ASSERT_EQ(msg.getFrameData(), payload);
    ASSERT_EQ(msg.getPayloadLength(), CANFD_MAX_DLEN);
    ASSERT_TRUE(msg.getRawFrame().can_id == 0x123 && msg.getRawFrame().len == CANFD_MAX_DLEN);
}

TEST(CanFdMessageTests, CanFdMessage_ConstructorWithTooMuchData_ExpectMessageSizeError) {
    const string payload(CANFD_MAX_DLEN + 1, 'a');

    ASSERT_THROW(CanFdMessage(0x123, payload), std::system_error);
}

TEST(CanFdMessageTests, CanFdMessage_ConstructorWithTimestamp_ExpectCorrectValues) {
    const auto timestamp = std::chrono::milliseconds(100);
    CanFdMessage msg(0x123, "TestData", timestamp);

    ASSERT_EQ(msg.getCanId(), 0x123);
    ASSERT_EQ(msg.getFrameData(), "TestData");
    ASSERT_EQ(msg.getTimestampOffset(), timestamp);
}

TEST(CanFdMessageTests, CanFdMessage_ConstructorWithFlags_ExpectFlagHelpers) {
    CanFdMessage msg(0x123, "TestData", static_cast<uint8_t>(CANFD_BRS | CANFD_ESI));

    ASSERT_EQ(msg.getFlags(), CANFD_BRS | CANFD_ESI);
    ASSERT_TRUE(msg.hasBitRateSwitch());
    ASSERT_TRUE(msg.hasErrorStateIndicator());
}

TEST(CanFdMessageTests, CanFdMessage_RawCanFdFrame_ExpectCorrectValues) {
    canfd_frame frame{};
    frame.can_id = 0x123 | CAN_EFF_FLAG;
    frame.len = 12;
    frame.flags = CANFD_BRS;
    std::copy_n(reinterpret_cast<const uint8_t*>("HelloWorld!!"), frame.len, frame.data);

    CanFdMessage msg(frame);

    ASSERT_EQ(msg.getCanId(), frame.can_id);
    ASSERT_EQ(msg.getFrameData(), "HelloWorld!!");
    ASSERT_EQ(msg.getPayloadLength(), 12);
    ASSERT_TRUE(msg.isExtendedFrameId());
    ASSERT_TRUE(msg.hasBitRateSwitch());
}

TEST(CanFdMessageTests, CanFdMessage_ClassicCanFrame_ExpectNormalizedFdFrame) {
    can_frame frame{};
    frame.can_id = 0x123;
    frame.can_dlc = 8;
    std::copy_n(reinterpret_cast<const uint8_t*>("TestData"), frame.can_dlc, frame.data);

    CanFdMessage msg(frame);

    ASSERT_EQ(msg.getCanId(), 0x123);
    ASSERT_EQ(msg.getFrameData(), "TestData");
    ASSERT_EQ(msg.getPayloadLength(), 8);
    ASSERT_EQ(msg.getFlags(), 0);
}

TEST(CanFdMessageTests, CanFdMessage_Equality_ExpectPayloadAndFlagsCompared) {
    CanFdMessage lhs(0x123, "TestData", static_cast<uint8_t>(CANFD_BRS));
    CanFdMessage rhs(0x123, "TestData", static_cast<uint8_t>(CANFD_BRS));
    CanFdMessage differentFlags(0x123, "TestData", static_cast<uint8_t>(CANFD_ESI));
    CanFdMessage differentPayload(0x123, "TestData2", static_cast<uint8_t>(CANFD_BRS));

    ASSERT_EQ(lhs, rhs);
    ASSERT_NE(lhs, differentFlags);
    ASSERT_NE(lhs, differentPayload);
}

TEST(CanFdMessageTests, CanDriver_StaticCanFdMaxLength_ExpectKernelConstant) {
    ASSERT_EQ(CanDriver::CANFD_MAX_DATA_LENGTH, CANFD_MAX_DLEN);
}

#if __cpp_lib_span >= 202002L
TEST(CanFdMessageTests, CanFdMessage_ConstructorWithSpan_ExpectCorrectValues) {
    CanFdMessage msg(0x123, std::span<const uint8_t>(reinterpret_cast<const uint8_t*>("TestData"), 8), static_cast<uint8_t>(CANFD_BRS));

    ASSERT_EQ(msg.getCanId(), 0x123);
    ASSERT_EQ(msg.getFrameData(), "TestData");
    ASSERT_EQ(msg.getPayloadLength(), 8);
    ASSERT_TRUE(msg.hasBitRateSwitch());
}
#endif // __cpp_lib_span >= 202002L
