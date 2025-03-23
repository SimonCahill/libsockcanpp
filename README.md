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

## Using the CAN driver

libsockcanpp provides a simple interface to socketcan, which is represented by the CanDriver class.<br >
`CanDriver` handles the setup of the socket; **it does not however setup the CAN interface!**

### Instantiating CanDriver

The example below describes how to instantiate a new instance of CanDriver.<br >
You can create as many instances as required, the resources are `free`'d once the instance has gone out of scope or been deleted.

The following parameters are required for correct instantiation:

1) CAN interface: [v]can[0-?]
2) CAN protocol: see [linux/can.h](https://github.com/linux-can/can-utils/blob/master/include/linux/can.h) for options
3) (optional) CAN sender ID (arbitration ID)

The following exceptions may be thrown if something went wrong during initialisation:

 - @see CanInitException
    - if socketcan failed to initlialise
    - if ioctl failed
    - if socket binding failed

CanDriver is fully thread-safe and can be used in multi-threaded applications.

```cpp
#include <CanDriver.hpp>

using sockcanpp::CanDriver;

int main() {
    CanDriver canDriver("can0", CAN_RAW[, 0xbad]);

    return 0;
}
```

### Using CAN IDs

libsockcanpp provides a first-class datatype, @see CanId, which acts as an integer which can be either 11 or 29 bits in size.<br >
The @see CanId type is used through libsockcanpp to provide a semi fool-proof method of using CAN arbitration IDs without the pitfalls of using traditional 32-bit integers.

CanId supports the following operations:
 - bitwise AND/OR
 - casting to: [u]int16, [u]int32
 - basic comparison:
    - equal to
    - not equal to
    - greater than (or equal to)
    - less than (or equal to)
 - arithmetic:
    - addition
    - subtraction

### Sending CAN frames

Sending CAN frames with sockcanpp is as easy as you could imagine.

1) instantiate a @see CanDriver object
2) create a @see CanMessage
3) send message

```cpp
#include <CanDriver.hpp>

using sockcanpp::CanDriver;
using sockcanpp::CanId;
using sockcanpp::CanMessage;

void sendCanFrameExample() {
    CanDriver canDriver("can0", CAN_RAW[, 0xd00d]);

    CanMessage messageToSend(0 /*send with default ID*/, "8 bytes!" /* the data */);

    auto sentByteCount = canDriver.sendMessage(messageToSend[, false /* don't force extended ID */]);

    printf("Sent %d bytes via CAN!\n", sentByteCount);
}

void sendMultipleFramesExample() {
    CanDriver canDriver("can1", CAN_RAW[, 0 /* no default ID */]);

    queue<CanMessage> messageQueue = {
        CanMessage(0x269, "somedata"),
        Canmessage(0x1e9, "moredata")
    };

    auto sentByteCount = canDriver.sendMessageQueue(messageQueue[, milliseconds(20) /* delay between frames */[, false /* don't force extended ID */]]);

    printf("Sent %d bytes via CAN!\n", sentByteCount);

}
```

### Receiving messages via CAN

Receiving CAN messages is almost as simple as sending them! Firstly, check if there are any messages in the buffer, then pull them out; either one-by-one, or all at once!

```cpp
#include <CanDriver.hpp>

using sockcanpp::CanDriver;
using sockcanpp::CanId;
using sockcanpp::CanMessage;

void receiveCanFramesExample() {
    CanDriver canDriver("can2", CAN_RAW[, 0 /* no default ID */]);

    if (canDriver.waitForMessages([milliseconds(3000) /* timeout */])) {
        // read a single message
        CanMessage receivedMessage = canDriver.readMessage();

        // read all available messages
        queue<CanMessage> receivedMessages = canDriver.readQueuedMessages();

        // handle CAN frames
    }
}
```
