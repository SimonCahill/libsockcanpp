/**
 * @file InterfaceSetupTest.cpp
 * @author Simon Cahill (s.cahill@procyon-systems.de)
 * @brief Contains the implementation of a basic test application using sockcanpp to set up a CAN interface.
 * @version 0.1
 * @date 2025-03-10
 * 
 * @copyright Copyright (c) 2025 Simon Cahill, Procyon Systems, and contributors.
 */

#include <CanDriver.hpp>
#include <exceptions/CanInitException.hpp>

#include <iostream>

using sockcanpp::CanDriver;
using sockcanpp::exceptions::CanInitException;

using std::cerr;
using std::cout;
using std::endl;
using std::string;

int main(int32_t argCount, char** argValues) {
    if (argCount < 2) {
        cerr << "Usage: " << argValues[0] << " <interface> <bitrate>" << endl;
        return 1;
    }

    const string interface{argValues[1]};
    const size_t bitrate = static_cast<size_t>(std::stoul(argValues[2]));

    try {
        CanDriver::setInterfaceUp(interface, bitrate);
    } catch (const CanInitException& ex) {
        cerr << "FAILED to set up interface: " << ex.getMessage() << endl;
        return 1;
    }

    cout << "Interface " << interface << " set up successfully!" << endl;

    return 0;
}