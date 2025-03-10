/**
 * @file InterfaceTests.cpp
 * @author Simon Cahill (s.cahill@procyon-systems.de)
 * @brief Contains multiple tests for the CanDriver class; specifically the new functionality allowing interfaces to be configured.
 * @version 0.1
 * @date 2025-03-10
 * 
 * @copyright Copyright (c) 2025 Simon Cahill, Procyon Systems, and contributors.
 */

#include <gtest/gtest.h>

#include <CanDriver.hpp>

#include <fstream>
#include <string>

using sockcanpp::CanDriver;

using std::ifstream;
using std::string;

static bool hasVcanModLoaded() {
    ifstream modulesFile{"/proc/modules"};
    string line{};

    while (getline(modulesFile, line)) {
        if (line.find("vcan") != string::npos) { return true; }
    }

    return false;
}

TEST(CanDriverTests, CanDriver_SetVcanInterfaceUp_ExpectTrue) {
    if (!hasVcanModLoaded()) { GTEST_SKIP(); }

    // Read /proc/net/dev to get a list of interfaces
    const auto readNetFile = []() {
        ifstream procNetDevFile{"/proc/net/dev"};
        string fContents{};

        string line{};
        while (getline(procNetDevFile, line)) { fContents += line + '\n'; }

        return fContents;
    };

    string netFileContents = readNetFile();
    ASSERT_FALSE(netFileContents.empty());
    ASSERT_TRUE(netFileContents.find("vcan0") == string::npos);

    ASSERT_NO_THROW(CanDriver::setInterfaceUp("vcan0", 500000));

    ASSERT_TRUE(netFileContents.find("vcan0") != string::npos);
}

TEST(CanDriverTests, CanDriver_SetAllCanInterfacesUp_ExpectNoThrow) {
    const auto interfaces = CanDriver::getAvailableInterfaces();

    if (interfaces.empty()) { GTEST_SKIP(); }

    for (const auto& iface : interfaces) {
        ASSERT_NO_THROW(CanDriver::setInterfaceUp(iface, 500000));
    }
}