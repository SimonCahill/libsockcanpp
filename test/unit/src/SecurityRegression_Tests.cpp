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
#include <sys/socket.h>
#include <unistd.h>

#include <string>
#include <system_error>

using sockcanpp::CanDriver;
using sockcanpp::CanFdMessage;
using sockcanpp::CanMessage;
using sockcanpp::exceptions::CanInitException;

#if defined(__SANITIZE_ADDRESS__)
#define LIBSOCKCANPP_HAS_ASAN 1
#elif defined(__has_feature)
#if __has_feature(address_sanitizer)
#define LIBSOCKCANPP_HAS_ASAN 1
#endif
#endif

TEST(SecurityRegressionTests, RawClassicCanMessageRejectsOversizedDlc) {
    can_frame frame{};
    frame.can_id = 0x123;
    frame.can_dlc = CAN_MAX_DLEN + 1;

    EXPECT_THROW({
        const CanMessage message(frame);
        static_cast<void>(message);
    }, std::system_error);
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

TEST(SecurityRegressionTests, RawCanFdMessageRejectsOversizedLength) {
    canfd_frame frame{};
    frame.can_id = 0x123;
    frame.len = CANFD_MAX_DLEN + 1;

    EXPECT_THROW({
        const CanFdMessage message(frame);
        static_cast<void>(message);
    }, std::system_error);
}

TEST(SecurityRegressionTests, OversizedInterfaceNameIsRejectedBeforeIfreqCopy) {
    const std::string oversizedInterfaceName(IFNAMSIZ, 'a');

#if defined(LIBSOCKCANPP_HAS_ASAN)
    const int probeSocket = socket(PF_CAN, SOCK_RAW, CanDriver::CAN_SOCK_RAW);
    if (probeSocket < 0) {
        GTEST_SKIP() << "Raw CAN sockets are not available in this environment";
    }
    close(probeSocket);

    EXPECT_DEATH({
        const CanDriver driver(oversizedInterfaceName, CanDriver::CAN_SOCK_RAW);
        static_cast<void>(driver);
    }, "");
#else
    EXPECT_THROW(CanDriver(oversizedInterfaceName, CanDriver::CAN_SOCK_RAW), CanInitException);
#endif
}
