# libsockcanpp

Welcome to the documentation for libsockcanpp!<br >
libsockcanpp is a socketcan wrapper library for C++, aimed to be easy to use without any fuss.

[![Build](https://github.com/SimonCahill/libsockcanpp/actions/workflows/cmake.yml/badge.svg)](https://github.com/SimonCahill/libsockcanpp/actions/workflows/cmake.yml)

# Useful Links

 - GitHub repo: [https://github.com/SimonCahill/libsockcanpp](https://github.com/SimonCahill/libsockcanpp)

# Getting Started

## Building

> [!NOTE]
> **C++11** SUPPORT:  
> This library supports modern C++ features, such as concepts in certain places.
> If your project cannot support C++20 features, you can force the C++ standard back by setting  
> `-Dsockcanpp_CONCEPT_SUPPORT=OFF` in the command-line or `set(sockcanpp_CONCEPT_SUPPORT OFF CACHE BOOL "Force C++ standard back to 11")`  
> in your CMakeLists.txt.

libsockcanpp was designed with use in CMake projects, but it can also easily be integrated into existing Makefile projects, as long as cmake is present on the build system.

1) clone the repository: `git clone https://github.com/SimonCahill/libsockcanpp.git`
2) create build directory: `mkdir build && cd build`
3) generate build files: `cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchains/desired-toolchain.cmake`
4) building library: `make -j`

## Incorporating into Cmake projects:

### Git Clone

1) clone the repository: `git clone https://github.com/SimonCahill/libsockcanpp.git`
2) add the following to CMakeLists.txt
```cmake
if (NOT TARGET sockcanpp)
    # IF you need C++11 support:
    # set(sockcanpp_CONCEPT_SUPPORT OFF CACHE BOOL "Force C++ standard back to 11")

    add_subdirectory(/path/to/libsockcanpprepo ${CMAKE_CURRENT_BUILD_DIR}/libsockcanpp)
endif()

# ... 

target_link_libraries(
    # ...
    sockcanpp
)
```
3) generate and build
4) ??? profit

### CPM
If your project utilises CPM, you can also run

```cmake
CPMAddPackage(
    NAME sockcanpp
    GIT_REPOSITORY https://github.com/SimonCahill/libsockcanpp.git
    GIT_TAG master
)
```

# Using libsockcanpp

## First Steps

In its current state, libsockcanpp doesn't support CANFD; very basic support exists for CANXL, although it is highly experimental and unstable.

This library provides a basic and straightforward API.
The main class is `CanDriver`; it is multi-instance and thread-safe.

### Setting up a CAN Socket

> [!NOTE]
> This library does **NOT** setup the CAN interface.  
> This can be done via the `ip` command:  
> `$ sudo ip link set dev can0 up type can bitrate <baud>`  
> `$ sudo ip link set dev van0 up type vcan`

```cpp
#include <CanDriver.hpp>

using sockcanpp::CanDriver;
using sockcanpp::CanMessage;
using sockcanpp::exceptions::CanException;

int main() {

    CanDriver cDriver{"can0", CAN_RAW}; // No default sender ID

    sendCanFrame(cDriver);
    sendMultipleFrames(cDriver);
    receiveFrame(cDriver);

    cDriver.setErrorFilter(); // Receive error frames

    filtermap_t canFilters{
        { 0x489,    0x7ff } // filter messages with 0x489 as ID
    };
    cDriver.setCanFilters(canFilters); // Set X amount of CAN filters. See https://docs.kernel.org/networking/can.html#raw-protocol-sockets-with-can-filters-sock-raw

    cDriver.setCollectTelemetry(); // Enable telemetry collection, such as timestamps, data sent, etc.

    cDriver.setReceiveOwnMessages(); // Echo sent frames back

    cDriver.setReturnRelativeTimestamps(); // Enable relative timestamps

}
```

### Sending a single CAN frame

```cpp
void sendCanFrame(CanDriver& driver) {
    CanMessage msg{0x123, "\x01\x02\x03\x04\x05\x06\x07\x08"};
    driver.sendMessage(msg);
    driver.sendMessage(msg, true); // force extended CAN frame
}
```

### Sending multiple CAN frames

```cpp
void sendMultipleFrames(CanDriver& driver) {
    vector<CanMessage> messages{
        CanMessage{0x123, "\x01\x02\x03\x04\x05\x06\x07\x08"},
        CanMessage{0x456, "\x09\x10\x11\x12\x13\x14\x15\x16"}
    };

    using namespace std::chrono::literals;

    driver.sendMessageQueue(messages); // 20ms delay (Default)
    driver.sendMessageQueue(messages, 20ns); // 20ns delay
    driver.sendMessageQueue(messages, 20us, true); // 20microseconds delay | force extended
}
```

### Receiving a frame

```cpp
void receiveFrame(CanDriver& driver) {
    if (!driver.waitForMessages()) { return; } // No messages in buffer

    const auto receivedMsg = driver.readMessage();
    std::cout << receivedMsg << std::endl; // Outputs: CanMessage(canId: XXX, data: FF FF FF FF, timestampOffset: Nms)

    if (receivedMsg.isErrorFrame()) {
        handleErrorFrame(receivedMsg);
    }
}
```

### Handling an Error Frame

libsockcanpp also provides features for evaluating errors states on the bus.  
To enable this feature, `cDriver.setErrorFilter();` needs to be enabled.

When an error frame is received, the information is provided by the `CanMessage` class.

```cpp
void handleErrorFrame(const CanMessage& msg) {

    // Handle as you see fit

    const auto hasBusError = msg.hasBusError();
    const auto hasBusOffError = msg.hasBusOffError();
    const auto hasControllerProblem = msg.hasControllerProblem();
    const auto hasControllerRestarted = msg.hasControllerRestarted();
    const auto hasErrorCounter = msg.hasErrorCounter();
    const auto hasLostArbitration = msg.hasLostArbitration();
    const auto hasProtocolViolation = msg.hasProtocolViolation();
    const auto hasTransceiverStatus = msg.hasTransceiverStatus();
    const auto missingAckOnTransmit = msg.missingAckOnTransmit();
    const auto isTxTimeout = msg.isTxTimeout();

    const auto controllerError = msg.getControllerError();
    const auto protocolError = msg.getProtocolError();
    const auto transceiverError = msg.getTransceiverError();
    const auto txErrorCounter = msg.getTxErrorCounter();
    const auto rxErrorCounter = msg.getRxErrorCounter();
    const auto arbitrationLostInBit = msg.arbitrationLostInBit();
}
```

© 2020–2025 Simon Cahill — Licensed under Apache License 2.0
