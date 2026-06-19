/**
 * @file SecurityRegression_Tests.cpp
 * @brief Regression tests for security-sensitive input validation.
 */

#include <gtest/gtest.h>

#include <CanDriver.hpp>
#include <CanFdMessage.hpp>
#include <CanMessage.hpp>
#include <exceptions/CanInitException.hpp>

#include <linux/can.h>
#include <net/if.h>

#include <algorithm>
#include <chrono>
#include <string>
#include <system_error>

using sockcanpp::CanDriver;
using sockcanpp::CanFdMessage;
using sockcanpp::CanMessage;
using sockcanpp::exceptions::CanInitException;

TEST(SecurityRegressionTests, RawClassicCanMessageAcceptsMaxDlc) {
    can_frame frame{};
    frame.can_id = 0x123;
    frame.can_dlc = CAN_MAX_DLEN;
    std::copy_n(reinterpret_cast<const uint8_t*>("12345678"), CAN_MAX_DLEN, frame.data);

    const CanMessage message(frame);

    EXPECT_EQ(message.getCanId(), 0x123);
    EXPECT_EQ(message.getFrameData(), "12345678");
}

TEST(SecurityRegressionTests, RawClassicCanMessageRejectsOversizedDlc) {
    can_frame frame{};
    frame.can_id = 0x123;
    frame.can_dlc = CAN_MAX_DLEN + 1;

    EXPECT_THROW({
        const CanMessage message(frame);
        static_cast<void>(message);
    }, std::system_error);
}

TEST(SecurityRegressionTests, RawClassicCanMessageWithTimestampRejectsOversizedDlc) {
    can_frame frame{};
    frame.can_id = 0x123;
    frame.can_dlc = CAN_MAX_DLEN + 1;

    EXPECT_THROW({
        const CanMessage message(frame, std::chrono::milliseconds(42));
        static_cast<void>(message);
    }, std::system_error);
}

TEST(SecurityRegressionTests, RawClassicCanFdMessageAcceptsMaxDlc) {
    can_frame frame{};
    frame.can_id = 0x123;
    frame.can_dlc = CAN_MAX_DLEN;
    std::copy_n(reinterpret_cast<const uint8_t*>("12345678"), CAN_MAX_DLEN, frame.data);

    const CanFdMessage message(frame);

    EXPECT_EQ(message.getCanId(), 0x123);
    EXPECT_EQ(message.getPayloadLength(), CAN_MAX_DLEN);
    EXPECT_EQ(message.getFrameData(), "12345678");
}

TEST(SecurityRegressionTests, RawClassicCanFdMessageRejectsOversizedDlc) {
    can_frame frame{};
    frame.can_id = 0x123;
    frame.can_dlc = CAN_MAX_DLEN + 1;

    EXPECT_THROW({
        const CanFdMessage message(frame);
        static_cast<void>(message);
    }, std::system_error);
}

TEST(SecurityRegressionTests, RawClassicCanFdMessageWithTimestampRejectsOversizedDlc) {
    can_frame frame{};
    frame.can_id = 0x123;
    frame.can_dlc = CAN_MAX_DLEN + 1;

    EXPECT_THROW({
        const CanFdMessage message(frame, std::chrono::milliseconds(42));
        static_cast<void>(message);
    }, std::system_error);
}

TEST(SecurityRegressionTests, RawCanFdMessageAcceptsMaxLength) {
    canfd_frame frame{};
    frame.can_id = 0x123;
    frame.len = CANFD_MAX_DLEN;
    std::fill_n(frame.data, CANFD_MAX_DLEN, static_cast<uint8_t>('x'));

    const CanFdMessage message(frame);

    EXPECT_EQ(message.getCanId(), 0x123);
    EXPECT_EQ(message.getPayloadLength(), CANFD_MAX_DLEN);
    EXPECT_EQ(message.getFrameData(), std::string(CANFD_MAX_DLEN, 'x'));
}

TEST(SecurityRegressionTests, RawCanFdMessageRejectsOversizedLength) {
    canfd_frame frame{};
    frame.can_id = 0x123;
    frame.len = CANFD_MAX_DLEN + 1;

    EXPECT_THROW({
        const CanFdMessage message(frame);
        static_cast<void>(message);
    }, std::system_error);
}

TEST(SecurityRegressionTests, RawCanFdMessageWithTimestampRejectsOversizedLength) {
    canfd_frame frame{};
    frame.can_id = 0x123;
    frame.len = CANFD_MAX_DLEN + 1;

    EXPECT_THROW({
        const CanFdMessage message(frame, std::chrono::milliseconds(42));
        static_cast<void>(message);
    }, std::system_error);
}

TEST(SecurityRegressionTests, OversizedInterfaceNameIsRejectedBeforeIfreqCopy) {
    const std::string oversizedInterfaceName(IFNAMSIZ, 'a');

    EXPECT_THROW(CanDriver(oversizedInterfaceName, CanDriver::CAN_SOCK_RAW), CanInitException);
}
