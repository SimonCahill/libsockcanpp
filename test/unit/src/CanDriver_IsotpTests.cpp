#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <memory>
#include <sockcanpp/CanDriver.hpp>
#include <string>
#include <thread>
#include <vector>

namespace {

    using sockcanpp::CanDriver;
    using sockcanpp::CanMessage;

    const sockcanpp::CanId REQUEST_ID{0x321};
    const sockcanpp::CanId RESPONSE_ID{0x322};

    std::string            interfaceName() {
        const char* value = std::getenv("SOCKCANPP_TEST_INTERFACE");
        return value == nullptr || *value == '\0' ? "vcan0" : value;
    }

    bool requireVcan() {
        const char* value = std::getenv("SOCKCANPP_REQUIRE_VCAN");
        return value != nullptr && std::string(value) == "1";
    }

    class IsoTpPeer {
        public:
            IsoTpPeer(const std::string& iface, sockcanpp::CanId tx, sockcanpp::CanId rx): driver_(iface, CanDriver::CAN_SOCK_RAW, {{rx, 0x7ff}}), tx_(tx), rx_(rx) { }

            void send(const std::vector<unsigned char>& payload) {
                EXPECT_FALSE(payload.empty());
                EXPECT_LE(payload.size(), 4095u);
                if (payload.empty() || payload.size() > 4095u) { return; }
                if (payload.size() <= 7u) {
                    std::string frame(1, static_cast<char>(payload.size()));
                    frame.append(payload.begin(), payload.end());
                    ASSERT_EQ(driver_.sendMessage(CanMessage(tx_, frame)), 8);
                    return;
                }

                std::string first(2, '\0');
                first[0] = static_cast<char>(0x10u | ((payload.size() >> 8u) & 0x0fu));
                first[1] = static_cast<char>(payload.size() & 0xffu);
                first.append(payload.begin(), payload.begin() + 6);
                ASSERT_EQ(driver_.sendMessage(CanMessage(tx_, first)), 8);

                const std::string flow = receiveFrame();
                ASSERT_GE(flow.size(), 3u);
                if (flow.size() < 3) { return; }
                EXPECT_EQ(static_cast<unsigned char>(flow[0]) & 0xf0u, 0x30u);
                EXPECT_EQ(static_cast<unsigned char>(flow[1]), 0u);
                if ((static_cast<unsigned char>(flow[0]) & 0xf0u) != 0x30u) { return; }

                size_t        offset   = 6;
                unsigned char sequence = 1;
                while (offset < payload.size()) {
                    const size_t count = std::min<size_t>(7, payload.size() - offset);
                    std::string  frame(1, static_cast<char>(0x20u | sequence));
                    frame.append(payload.begin() + offset, payload.begin() + offset + count);
                    EXPECT_EQ(driver_.sendMessage(CanMessage(tx_, frame)), 1 + static_cast<ssize_t>(count));
                    offset += count;
                    sequence = static_cast<unsigned char>((sequence + 1u) & 0x0fu);
                }
            }

            std::vector<unsigned char> receive() {
                const std::string first = receiveFrame();
                if (first.empty()) { return std::vector<unsigned char>(); }
                const unsigned char type = static_cast<unsigned char>(first[0]) & 0xf0u;
                if (type == 0x00u) {
                    const size_t length = static_cast<unsigned char>(first[0]) & 0x0fu;
                    return std::vector<unsigned char>(first.begin() + 1, first.begin() + 1 + length);
                }

                EXPECT_EQ(type, 0x10u);
                if (first.size() < 2 || type != 0x10u) { return std::vector<unsigned char>(); }
                const size_t               length = ((static_cast<unsigned char>(first[0]) & 0x0fu) << 8u) | static_cast<unsigned char>(first[1]);
                std::vector<unsigned char> result(first.begin() + 2, first.end());
                EXPECT_LE(result.size(), length);
                EXPECT_EQ(driver_.sendMessage(CanMessage(tx_, std::string("\x30\x00\x00", 3))), 3);

                unsigned char sequence = 1;
                while (result.size() < length) {
                    const std::string frame = receiveFrame();
                    EXPECT_EQ(static_cast<unsigned char>(frame[0]) & 0xf0u, 0x20u);
                    EXPECT_EQ(static_cast<unsigned char>(frame[0]) & 0x0fu, sequence);
                    result.insert(result.end(), frame.begin() + 1, frame.end());
                    sequence = static_cast<unsigned char>((sequence + 1u) & 0x0fu);
                }
                result.resize(length);
                return result;
            }

        private:
            std::string receiveFrame() {
                if (!driver_.waitForMessages(std::chrono::milliseconds(500))) {
                    ADD_FAILURE() << "Timed out waiting for CAN frame on " << interfaceName();
                    return std::string();
                }
                const CanMessage message = driver_.readMessage();
                EXPECT_EQ(message.getCanId(), rx_);
                return message.getFrameData();
            }

            CanDriver        driver_;
            sockcanpp::CanId tx_;
            sockcanpp::CanId rx_;
    };

    class CanDriverIsotpTest: public ::testing::Test {
        protected:
            void SetUp() override {
                try {
                    sender_.reset(new IsoTpPeer(interfaceName(), REQUEST_ID, RESPONSE_ID));
                    receiver_.reset(new IsoTpPeer(interfaceName(), RESPONSE_ID, REQUEST_ID));
                } catch (const std::exception& error) {
                    if (requireVcan()) { FAIL() << "Unable to open " << interfaceName() << ": " << error.what(); }
                    GTEST_SKIP() << "ISO-TP communication tests require " << interfaceName() << ": " << error.what();
                }
            }

            std::unique_ptr<IsoTpPeer> sender_;
            std::unique_ptr<IsoTpPeer> receiver_;
    };

    TEST_F(CanDriverIsotpTest, SingleFrameRoundTrip) {
        const std::vector<unsigned char> payload{'o', 'k'};
        sender_->send(payload);
        EXPECT_EQ(receiver_->receive(), payload);
    }

    TEST_F(CanDriverIsotpTest, MultiFramePayloadUsesFlowControlAndReassembles) {
        std::vector<unsigned char> payload(134);
        for (size_t i = 0; i < payload.size(); ++i) { payload[i] = static_cast<unsigned char>(i); }
        // The receiver must process the FF/FC exchange before the sender continues.
        // Run the sender's frame production after the receiver has accepted the FF.
        ASSERT_TRUE(sender_ != nullptr && receiver_ != nullptr);
        std::thread tx([&] { sender_->send(payload); });
        EXPECT_EQ(receiver_->receive(), payload);
        tx.join();
    }

} // namespace
