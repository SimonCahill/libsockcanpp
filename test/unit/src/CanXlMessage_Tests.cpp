#ifdef SOCKCANPP_CANXL_SUPPORT

#include <gtest/gtest.h>

#include <sockcanpp/CanDriver.hpp>
#include <sockcanpp/CanXlMessage.hpp>

#include <linux/can.h>

#include <chrono>
#include <string>
#include <system_error>

using sockcanpp::CanDriver;
using sockcanpp::CanXlMessage;

TEST(CanXlMessageTests, ConstructorSetsHeaderAndPayload) {
    const std::string payload(128, 'x');
    #ifdef CANXL_VCID_OFFSET
    constexpr uint8_t virtualCanId = 0x45;
    #else
    constexpr uint8_t virtualCanId = 0;
    #endif
    CanXlMessage message(0x321, payload, 0x12, 0x12345678, CANXL_SEC, virtualCanId);

    EXPECT_EQ(message.getPriority(), 0x321);
    EXPECT_EQ(message.getVirtualCanId(), virtualCanId);
    EXPECT_EQ(message.getPayloadLength(), payload.size());
    EXPECT_EQ(message.getFrameData(), payload);
    EXPECT_EQ(message.getSduType(), 0x12);
    EXPECT_EQ(message.getAcceptanceField(), 0x12345678U);
    EXPECT_TRUE(message.getFlags() & CANXL_XLF);
    EXPECT_TRUE(message.hasSimpleExtendedContent());
    EXPECT_FALSE(message.hasRemoteRequestSubstitution());
}

TEST(CanXlMessageTests, ConstructorRejectsInvalidPayloadLengths) {
    EXPECT_THROW(CanXlMessage(0x123, std::string{}), std::system_error);
    EXPECT_THROW(CanXlMessage(0x123, std::string(CANXL_MAX_DLEN + 1, 'x')), std::system_error);
}

TEST(CanXlMessageTests, ConstructorRejectsPriorityLargerThanElevenBits) {
    EXPECT_THROW(CanXlMessage(CANXL_PRIO_MASK + 1, "x"), std::system_error);
}

TEST(CanXlMessageTests, RawFrameAndTimestampArePreserved) {
    canxl_frame frame{};
    frame.prio = 0x456;
    frame.flags = CANXL_XLF;
    #ifdef CANXL_RRS
    frame.flags |= CANXL_RRS;
    #endif
    frame.sdt = 0x22;
    frame.len = 4;
    frame.af = 0xabcdef01;
    std::copy_n("data", 4, frame.data);

    const auto timestamp = std::chrono::milliseconds(42);
    CanXlMessage message(frame, timestamp);

    EXPECT_EQ(message.getRawFrame().af, frame.af);
    EXPECT_EQ(message.getFrameData(), "data");
    EXPECT_EQ(message.getTimestampOffset(), timestamp);
    #ifdef CANXL_RRS
    EXPECT_TRUE(message.hasRemoteRequestSubstitution());
    #else
    EXPECT_FALSE(message.hasRemoteRequestSubstitution());
    #endif
}

TEST(CanXlMessageTests, EqualityIncludesHeaderAndPayload) {
    CanXlMessage lhs(0x123, "payload", 1, 2, CANXL_SEC);
    CanXlMessage rhs(0x123, "payload", 1, 2, CANXL_SEC);
    CanXlMessage different(0x123, "payload", 1, 4, CANXL_SEC);

    EXPECT_EQ(lhs, rhs);
    EXPECT_NE(lhs, different);
}

#ifndef CANXL_VCID_OFFSET
TEST(CanXlMessageTests, VcidRequiresKernelHeaderSupport) {
    EXPECT_THROW(CanXlMessage(0x123, "payload", 0, 0, 0, 1), std::system_error);
}
#endif

TEST(CanXlMessageTests, DriverMaximumLengthMatchesKernelAbi) {
    EXPECT_EQ(CanDriver::CANXL_MAX_DATA_LENGTH, CANXL_MAX_DLEN);
}

#if __cpp_lib_span >= 201907L
TEST(CanXlMessageTests, SpanConstructorCopiesPayload) {
    const uint8_t payload[]{1, 2, 3, 4};
    CanXlMessage message(0x123, std::span<const uint8_t>(payload));

    EXPECT_EQ(message.getPayloadLength(), 4U);
    EXPECT_EQ(message.getRawFrame().data[3], 4);
}
#endif

#endif // SOCKCANPP_CANXL_SUPPORT
