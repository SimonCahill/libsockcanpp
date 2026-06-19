/**
 * @file MessageFuzzer.cpp
 * @brief Fuzzes raw CAN frame constructors and accessors.
 */

#include <CanFdMessage.hpp>
#include <CanMessage.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <linux/can.h>
#include <stdexcept>

namespace {

template<typename Frame>
void copyInput(const uint8_t* data, size_t size, Frame& frame) {
    const auto bytesToCopy = std::min(size, sizeof(Frame));
    std::memcpy(&frame, data, bytesToCopy);
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (data == nullptr || size == 0) {
        return 0;
    }

    try {
        can_frame classicFrame{};
        copyInput(data, size, classicFrame);

        const sockcanpp::CanMessage classicMessage(classicFrame);
        static_cast<void>(classicMessage.getFrameData());
        static_cast<void>(classicMessage == classicMessage);

        const sockcanpp::CanFdMessage fdFromClassic(classicFrame);
        static_cast<void>(fdFromClassic.getFrameData());
        static_cast<void>(fdFromClassic == fdFromClassic);

        canfd_frame fdFrame{};
        copyInput(data, size, fdFrame);

        const sockcanpp::CanFdMessage fdMessage(fdFrame);
        static_cast<void>(fdMessage.getFrameData());
        static_cast<void>(fdMessage == fdMessage);
    } catch (const std::exception&) {
        return 0;
    }

    return 0;
}
