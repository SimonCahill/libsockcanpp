/**
 * @file CanMessage_Tests.cpp
 * @author Simon Cahill (s.cahill@procyon-systems.de)
 * @brief Contains all the unit tests for the CanMessage structure.
 * @version 0.1
 * @date 2025-07-30
 * 
 * @copyright Copyright (c) 2025 Simon Cahill and Contributors.
 */

#include <gtest/gtest.h>

#include <CanMessage.hpp>

using sockcanpp::CanMessage;

using std::string;

TEST(CanMessageTests, CanMessage_Constructor_ExpectDefaultValues) {
    CanMessage msg;

    ASSERT_EQ(msg.getCanId(), 0);
    ASSERT_EQ(msg.getFrameData(), "");
    ASSERT_TRUE(msg.getRawFrame().can_id == 0 && msg.getRawFrame().can_dlc == 0);
}

TEST(CanMessageTests, CanMessage_ConstructorWithId_ExpectCorrectId) {
    CanMessage msg(0x123, "");

    ASSERT_EQ(msg.getCanId(), 0x123);
    ASSERT_EQ(msg.getFrameData(), "");
    ASSERT_TRUE(msg.getRawFrame().can_id == 0x123 && msg.getRawFrame().can_dlc == 0);
}

TEST(CanMessageTests, CanMessage_ConstructorWithId_ExpectCorrectIdAndTestData) {
    CanMessage msg(0x123, "TestData");

    ASSERT_EQ(msg.getCanId(), 0x123);
    ASSERT_EQ(msg.getFrameData(), "TestData");
    ASSERT_TRUE(msg.getRawFrame().can_id == 0x123 && msg.getRawFrame().can_dlc == 8);
}

TEST(CanMessageTests, CanMessage_ConstructorWithIdAndTimestamp_ExpectCorrectValues) {
    auto timestamp = std::chrono::milliseconds(100);
    CanMessage msg(0x123, "TestData", timestamp);

    ASSERT_EQ(msg.getCanId(), 0x123);
    ASSERT_EQ(msg.getFrameData(), "TestData");
    ASSERT_TRUE(msg.getRawFrame().can_id == 0x123 && msg.getRawFrame().can_dlc == 8);
    ASSERT_EQ(msg.getTimestampOffset(), timestamp);
}

#if __cpp_lib_span >= 202002L
TEST(CanMessageTests, CanMessage_ConstructorWithIdAndSpan_ExpectCorrectValues) {
    CanMessage msg(0x123, std::span<const uint8_t>(reinterpret_cast<const uint8_t*>("TestData"), 8));

    ASSERT_EQ(msg.getCanId(), 0x123);
    ASSERT_EQ(msg.getFrameData(), "TestData");
    ASSERT_TRUE(msg.getRawFrame().can_id == 0x123 && msg.getRawFrame().can_dlc == 8);
}
#endif // __cpp_lib_span >= 202002L