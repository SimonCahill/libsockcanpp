/**
 * @file Main.cpp
 * @author Simon Cahill (contact@simonc.eu)
 * @brief CAN/CAN FD/CAN XL responder test application.
 * @version 0.1
 * @date 2026-06-17
 *
 * @copyright Copyright (c) 2026 Simon Cahill
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/select.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>

#include <CanDriver.hpp>
#include <CanFdMessage.hpp>
#include <CanMessage.hpp>
#include <exceptions/CanException.hpp>
#include <exceptions/CanInitException.hpp>
#include <exceptions/InvalidSocketException.hpp>

using sockcanpp::CanDriver;
using sockcanpp::CanFdMessage;
using sockcanpp::CanId;
using sockcanpp::CanMessage;
using sockcanpp::exceptions::CanException;
using sockcanpp::exceptions::CanInitException;
using sockcanpp::exceptions::InvalidSocketException;

namespace {

    enum class FrameKind {
        Can,
        CanFd,
        CanXl
    };

    struct Options {
        bool starter{false};
        bool autoIncrementIds{true};
        std::string bus{"can0"};
        canid_t canId{0x123};
        canid_t canFdId{0x456};
        canid_t canXlId{0x789};
        uint32_t count{0};
        uint32_t intervalMs{250};
    };

    class RandomData {
        public:
            RandomData(): m_engine(std::random_device{}()) { }

            std::string bytes(size_t minLength, size_t maxLength) {
                std::uniform_int_distribution<size_t> lengthDistribution(minLength, maxLength);
                std::uniform_int_distribution<uint16_t> byteDistribution(0, 255);

                const auto length = lengthDistribution(m_engine);
                std::string data(length, '\0');
                std::generate(data.begin(), data.end(), [&]() {
                    return static_cast<char>(byteDistribution(m_engine));
                });

                return data;
            }

        private:
            std::mt19937 m_engine;
    };

    void printHelp(const char* appName) {
        std::cout
            << appName << "\n\n"
            << "Modes:\n"
            << "  -s, --starter             Send initial CAN, CAN FD and CAN XL frames, then respond\n"
            << "                            to received frames. Without this flag the app only listens\n"
            << "                            and responds.\n\n"
            << "Options:\n"
            << "  -b, --bus=canX            SocketCAN bus to use (default: can0)\n"
            << "      --bus canX\n"
            << "  -i, --id=HEX              Set all transmit IDs and keep them fixed unless --auto-id\n"
            << "                            is also supplied\n"
            << "      --can-id=HEX          Set classic CAN transmit ID\n"
            << "      --fd-id=HEX           Set CAN FD transmit ID\n"
            << "      --xl-id=HEX           Set CAN XL transmit priority/ID\n"
            << "      --auto-id             Increment each frame family's ID after every send (default)\n"
            << "      --no-auto-id          Reuse configured IDs for every send\n"
            << "  -n, --count=N             Stop after N sends. 0 means run forever (default)\n"
            << "      --interval-ms=N       Delay between starter seed frames (default: 250)\n"
            << "  -h, --help                Print this menu\n";
    }

    bool startsWith(const std::string& value, const std::string& prefix) {
        return value.compare(0, prefix.size(), prefix) == 0;
    }

    std::string requireValue(int32_t& index, int32_t argCount, char** argValues, const std::string& option) {
        if (index + 1 >= argCount) {
            throw std::invalid_argument(option + " requires a value");
        }

        ++index;
        return argValues[index];
    }

    uint32_t parseNumber(const std::string& value) {
        size_t parsedChars{0};
        const auto number = std::stoul(value, &parsedChars, 0);
        if (parsedChars != value.size()) {
            throw std::invalid_argument("Invalid numeric value: " + value);
        }

        return static_cast<uint32_t>(number);
    }

    canid_t parseCanId(const std::string& value) {
        size_t parsedChars{0};
        const auto id = std::stoul(value, &parsedChars, 16);
        if (parsedChars != value.size()) {
            throw std::invalid_argument("Invalid CAN ID: " + value);
        }

        if (id > CAN_EFF_MASK) {
            throw std::invalid_argument("CAN ID out of range: " + value);
        }

        return static_cast<canid_t>(id);
    }

    Options parseOptions(int32_t argCount, char** argValues) {
        Options options{};

        for (int32_t i = 1; i < argCount; ++i) {
            const std::string arg{argValues[i]};

            if (arg == "-h" || arg == "--help") {
                printHelp(argValues[0]);
                std::exit(0);
            } else if (arg == "-s" || arg == "--starter") {
                options.starter = true;
            } else if (arg == "-b" || arg == "--bus") {
                options.bus = requireValue(i, argCount, argValues, arg);
            } else if (startsWith(arg, "--bus=")) {
                options.bus = arg.substr(std::strlen("--bus="));
            } else if (arg == "-i" || arg == "--id") {
                const auto id = parseCanId(requireValue(i, argCount, argValues, arg));
                options.canId = id;
                options.canFdId = id;
                options.canXlId = id;
                options.autoIncrementIds = false;
            } else if (startsWith(arg, "--id=")) {
                const auto id = parseCanId(arg.substr(std::strlen("--id=")));
                options.canId = id;
                options.canFdId = id;
                options.canXlId = id;
                options.autoIncrementIds = false;
            } else if (arg == "--can-id") {
                options.canId = parseCanId(requireValue(i, argCount, argValues, arg));
                options.autoIncrementIds = false;
            } else if (startsWith(arg, "--can-id=")) {
                options.canId = parseCanId(arg.substr(std::strlen("--can-id=")));
                options.autoIncrementIds = false;
            } else if (arg == "--fd-id") {
                options.canFdId = parseCanId(requireValue(i, argCount, argValues, arg));
                options.autoIncrementIds = false;
            } else if (startsWith(arg, "--fd-id=")) {
                options.canFdId = parseCanId(arg.substr(std::strlen("--fd-id=")));
                options.autoIncrementIds = false;
            } else if (arg == "--xl-id") {
                options.canXlId = parseCanId(requireValue(i, argCount, argValues, arg));
                options.autoIncrementIds = false;
            } else if (startsWith(arg, "--xl-id=")) {
                options.canXlId = parseCanId(arg.substr(std::strlen("--xl-id=")));
                options.autoIncrementIds = false;
            } else if (arg == "--auto-id") {
                options.autoIncrementIds = true;
            } else if (arg == "--no-auto-id") {
                options.autoIncrementIds = false;
            } else if (arg == "-n" || arg == "--count") {
                options.count = parseNumber(requireValue(i, argCount, argValues, arg));
            } else if (startsWith(arg, "--count=")) {
                options.count = parseNumber(arg.substr(std::strlen("--count=")));
            } else if (arg == "--interval-ms") {
                options.intervalMs = parseNumber(requireValue(i, argCount, argValues, arg));
            } else if (startsWith(arg, "--interval-ms=")) {
                options.intervalMs = parseNumber(arg.substr(std::strlen("--interval-ms=")));
            } else {
                throw std::invalid_argument("Unknown argument: " + arg);
            }
        }

        return options;
    }

    void incrementId(canid_t& id) {
        id = (id + 1) & CAN_EFF_MASK;
        if (id == 0) {
            id = 1;
        }
    }

    canid_t nextId(canid_t& id, bool autoIncrement) {
        const auto currentId = id;
        if (autoIncrement) {
            incrementId(id);
        }

        return currentId;
    }

    bool shouldStop(uint32_t sentFrames, const Options& options) {
        return options.count > 0 && sentFrames >= options.count;
    }

    void printBytes(const std::string& data) {
        for (const auto byte : data) {
            std::cout << std::hex << std::setw(2) << std::setfill('0')
                      << (static_cast<uint16_t>(static_cast<uint8_t>(byte))) << " ";
        }
        std::cout << std::dec << std::setfill(' ');
    }

    void logSend(FrameKind kind, canid_t id, const std::string& data) {
        const char* name = kind == FrameKind::Can ? "CAN" : (kind == FrameKind::CanFd ? "CAN FD" : "CAN XL");
        std::cout << "Sent " << name << " id=0x" << std::hex << id << std::dec
                  << " len=" << data.size() << " data=";
        printBytes(data);
        std::cout << "\n";
    }

    void sendCan(CanDriver& driver, Options& options, RandomData& randomData) {
        const auto id = nextId(options.canId, options.autoIncrementIds);
        const auto payload = randomData.bytes(0, CAN_MAX_DLEN);
        driver.sendMessage(CanMessage(CanId(id), payload), id > CAN_SFF_MASK);
        logSend(FrameKind::Can, id, payload);
    }

    void sendCanFd(CanDriver& driver, Options& options, RandomData& randomData) {
        const auto id = nextId(options.canFdId, options.autoIncrementIds);
        const auto payload = randomData.bytes(0, CANFD_MAX_DLEN);
        driver.sendCanFdMessage(CanFdMessage(CanId(id), payload, CANFD_BRS), id > CAN_SFF_MASK);
        logSend(FrameKind::CanFd, id, payload);
    }

    void sendCanXl(CanDriver& driver, Options& options, RandomData& randomData) {
        #ifdef CANXL_XLF
            const auto id = nextId(options.canXlId, options.autoIncrementIds);
            const auto payload = randomData.bytes(CANXL_MIN_DLEN, std::min<size_t>(CANXL_MAX_DLEN, 64));

            canxl_frame frame{};
            frame.prio = id & CANXL_PRIO_MASK;
            frame.flags = CANXL_XLF;
            frame.len = payload.size();
            std::copy(payload.begin(), payload.end(), frame.data);

            const auto bytesWritten = write(driver.getSocketFd(), &frame, CANXL_MTU);
            if (bytesWritten < 0) {
                throw CanException(
                    sockcanpp::formatString("FAILED to write CAN XL data! Error: %d => %s", errno, strerror(errno)),
                    driver.getSocketFd()
                );
            }

            logSend(FrameKind::CanXl, id, payload);
        #else
            static_cast<void>(driver);
            static_cast<void>(options);
            static_cast<void>(randomData);
            std::cout << "Skipping CAN XL send: kernel headers do not provide CAN XL support\n";
        #endif
    }

    uint32_t sendFrame(CanDriver& driver, Options& options, RandomData& randomData, FrameKind kind, uint32_t sentFrames) {
        if (shouldStop(sentFrames, options)) {
            return sentFrames;
        }

        switch (kind) {
            case FrameKind::Can:
                sendCan(driver, options, randomData);
                break;
            case FrameKind::CanFd:
                sendCanFd(driver, options, randomData);
                break;
            case FrameKind::CanXl:
                sendCanXl(driver, options, randomData);
                break;
        }

        return sentFrames + 1;
    }

    uint32_t sendStarterFrames(CanDriver& driver, Options& options, RandomData& randomData, uint32_t sentFrames) {
        sentFrames = sendFrame(driver, options, randomData, FrameKind::Can, sentFrames);
        std::this_thread::sleep_for(std::chrono::milliseconds(options.intervalMs));

        sentFrames = sendFrame(driver, options, randomData, FrameKind::CanFd, sentFrames);
        std::this_thread::sleep_for(std::chrono::milliseconds(options.intervalMs));

        sentFrames = sendFrame(driver, options, randomData, FrameKind::CanXl, sentFrames);
        return sentFrames;
    }

    bool waitForSocket(int32_t socketFd, std::chrono::milliseconds timeout) {
        fd_set readFileDescriptors;
        FD_ZERO(&readFileDescriptors);
        FD_SET(socketFd, &readFileDescriptors);

        timeval waitTime{};
        waitTime.tv_sec = static_cast<time_t>(timeout.count() / 1000);
        waitTime.tv_usec = static_cast<suseconds_t>((timeout.count() % 1000) * 1000);

        return select(socketFd + 1, &readFileDescriptors, nullptr, nullptr, &waitTime) > 0;
    }

    FrameKind readFrame(int32_t socketFd) {
        alignas(canfd_frame) uint8_t frameBuffer[
        #ifdef CANXL_MTU
            CANXL_MTU
        #else
            CANFD_MTU
        #endif
        ]{};

        const auto bytesRead = read(socketFd, frameBuffer, sizeof(frameBuffer));
        if (bytesRead < 0) {
            throw CanException(
                sockcanpp::formatString("FAILED to read frame! Error: %d => %s", errno, strerror(errno)),
                socketFd
            );
        }

        if (bytesRead == CAN_MTU) {
            const auto* frame = reinterpret_cast<const can_frame*>(frameBuffer);
            std::string data(frame->data, frame->data + frame->can_dlc);
            std::cout << "Received CAN id=0x" << std::hex << (frame->can_id & CAN_EFF_MASK) << std::dec
                      << " len=" << data.size() << " data=";
            printBytes(data);
            std::cout << "\n";
            return FrameKind::Can;
        }

        if (bytesRead == CANFD_MTU) {
            const auto* frame = reinterpret_cast<const canfd_frame*>(frameBuffer);
            std::string data(frame->data, frame->data + frame->len);
            std::cout << "Received CAN FD id=0x" << std::hex << (frame->can_id & CAN_EFF_MASK) << std::dec
                      << " len=" << data.size() << " data=";
            printBytes(data);
            std::cout << "\n";
            return FrameKind::CanFd;
        }

        #ifdef CANXL_XLF
            if (bytesRead >= static_cast<ssize_t>(CANXL_HDR_SIZE)) {
                const auto* frame = reinterpret_cast<const canxl_frame*>(frameBuffer);
                if (frame->flags & CANXL_XLF) {
                    std::string data(frame->data, frame->data + frame->len);
                    std::cout << "Received CAN XL id=0x" << std::hex << (frame->prio & CANXL_PRIO_MASK) << std::dec
                              << " len=" << data.size() << " data=";
                    printBytes(data);
                    std::cout << "\n";
                    return FrameKind::CanXl;
                }
            }
        #endif

        throw CanException(
            sockcanpp::formatString("FAILED to read frame! Unknown MTU: %zd bytes", bytesRead),
            socketFd
        );
    }

    void configureSocket(CanDriver& driver) {
        driver.allowCanFdFrames(true);

        #ifdef CANXL_XLF
            driver.allowCanXlFrames(true);
        #endif
    }
}

int main(int32_t argCount, char** argValues) {
    Options options{};

    try {
        options = parseOptions(argCount, argValues);
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << "\n\n";
        printHelp(argValues[0]);
        return 1;
    }

    try {
        CanDriver driver(options.bus, CanDriver::CAN_SOCK_RAW);
        configureSocket(driver);

        RandomData randomData{};
        uint32_t sentFrames{0};

        if (options.starter) {
            sentFrames = sendStarterFrames(driver, options, randomData, sentFrames);
        }

        while (!shouldStop(sentFrames, options)) {
            if (!waitForSocket(driver.getSocketFd(), std::chrono::milliseconds(1000))) {
                continue;
            }

            const auto frameKind = readFrame(driver.getSocketFd());
            sentFrames = sendFrame(driver, options, randomData, frameKind, sentFrames);
        }

        return 0;
    } catch (const CanInitException& ex) {
        std::cerr << "Failed to initialise CAN driver: " << ex.what() << "\n";
    } catch (const InvalidSocketException& ex) {
        std::cerr << "Invalid CAN socket: " << ex.what() << "\n";
    } catch (const CanException& ex) {
        std::cerr << "CAN error: " << ex.what() << "\n";
    } catch (const std::exception& ex) {
        std::cerr << "Unhandled error: " << ex.what() << "\n";
    }

    return 1;
}
