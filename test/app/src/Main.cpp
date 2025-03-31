/**
 * @file Main.cpp
 * @author Simon Cahill (contact@simonc.eu)
 * @brief Contains the implementation of a test application using this library.
 * @version 0.1
 * @date 2020-07-02
 * 
 * @copyright Copyright (c) 2020 Simon Cahill
 *
 *  Copyright 2020 Simon Cahill
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

#include <iostream>
#include <string>

#include <CanDriver.hpp>
#include <exceptions/CanException.hpp>
#include <exceptions/CanInitException.hpp>
#include <exceptions/InvalidSocketException.hpp>

using namespace std::chrono_literals;

using sockcanpp::CanDriver;
using sockcanpp::CanId;
using sockcanpp::exceptions::CanException;
using sockcanpp::exceptions::CanInitException;
using sockcanpp::exceptions::InvalidSocketException;
using sockcanpp::CanMessage;

using std::cerr;
using std::cout;
using std::endl;
using std::string;

void printHelp(string);

int main(int32_t argCount, char** argValues) {
    int32_t desiredCanSocket = 0;
    string canInterface;

    if (argCount > 2) {
        for (int32_t i = 1; i < argCount; i++) {
            string arg{argValues[i]};
            if (arg == "--help" || arg == "-h") {
                printHelp(argValues[0]);
                return 0;
            } else if (arg == "-protocol") {
                desiredCanSocket = atoi(argValues[i + 1]);
                i += 1;
                continue;
            } else if (arg == "-iface") {
                canInterface = (argValues[i + 1]);
                i += 1;
                continue;
            }
        }
    }

    if (desiredCanSocket <= 0)
        desiredCanSocket = CanDriver::CAN_SOCK_RAW;
    if (canInterface == "")
        canInterface = "can0";

    CanDriver* canDriver{nullptr};
    try {
        canDriver = new CanDriver(canInterface, desiredCanSocket);
    } catch (CanInitException& ex) {
        cerr << "An error occurred while initialising CanDriver: " << ex.what() << endl;
        if (canDriver) { delete canDriver; }
        return -1;
    }

    while (true) {
        printf("Writing test message:\n");
        try { canDriver->sendMessage(CanMessage(0x555, "abcdefg8")); }
        catch (CanException& ex) { cerr << "Failed to send test message! " << ex.what() << endl; }
        catch (InvalidSocketException& ex) { cerr << "Failed to send test message! " << ex.what() << endl; }

        printf("Reading messages\n");
        if (!canDriver->waitForMessages(3000ns)) continue;

        cout << "Reading queue..." << endl;
        auto canMessages = canDriver->readQueuedMessages();
        while (!canMessages.empty()) {
            auto msg = canMessages.front();
            canMessages.pop();

            cout << "CAN ID: " << (int32_t)msg.getCanId() << endl
                 << "CAN data: ";
            for (auto byte : msg.getFrameData())
                cout << std::hex << byte << " ";
            cout << endl;
        }
    }
}

void printHelp(string appname) {
    cout << appname << endl << endl
         << "-h\t\tPrints this menu" << endl
         << "--help\t\tPrints this menu" << endl
         << "-protocol <protocol_num>" << endl
         << "-iface <can_iface>" << endl;
}
