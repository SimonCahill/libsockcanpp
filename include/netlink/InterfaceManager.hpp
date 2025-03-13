/**
 * @file InterfaceManager.hpp
 * @author Simon Cahill (s.cahill@procyon-systems.de)
 * @brief Contains the declaration and implementation of the InterfaceManager class, which communicates with the Kernel's netlink interface and fetches information about CAN interfaces.
 * @version 0.1
 * @date 2025-03-11
 * 
 * @copyright Copyright (c) 2025 Procyon Systems, Simon Cahill and Contributors.
 */

#ifndef LIBSOCKCANPP_INCLUDE_NETLINK_INTERFACEMANAGER_HPP
#define LIBSOCKCANPP_INCLUDE_NETLINK_INTERFACEMANAGER_HPP

#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <net/if.h>
#include <linux/rtnetlink.h>
#include <linux/if_arp.h>
#include <libmnl/libmnl.h>

// Define a buffer size for our netlink socket messages.
#ifndef MNL_SOCKET_BUFFER_SIZE
#define MNL_SOCKET_BUFFER_SIZE 8192
#endif

class SocketCANManager {
public:
    SocketCANManager();
    ~SocketCANManager();

    // Fetch list of SocketCAN interfaces (interfaces whose ifi_type equals ARPHRD_CAN)
    std::vector<std::string> getSocketCANInterfaces();

    // Bring the interface up
    bool setInterfaceUp(const std::string& ifName);

    // Bring the interface down
    bool setInterfaceDown(const std::string& ifName);

private:
    struct mnl_socket* nlSocket;
    int seq;

    // Callback to process each netlink message
    static int data_cb(const struct nlmsghdr *nlh, void *data);
};

SocketCANManager::SocketCANManager() {
    nlSocket = mnl_socket_open(NETLINK_ROUTE);
    if (nlSocket == nullptr) {
        std::cerr << "Error: mnl_socket_open failed." << std::endl;
        exit(EXIT_FAILURE);
    }
    if (mnl_socket_bind(nlSocket, 0, MNL_SOCKET_AUTOPID) < 0) {
        std::cerr << "Error: mnl_socket_bind failed." << std::endl;
        mnl_socket_close(nlSocket);
        exit(EXIT_FAILURE);
    }
    // Use current time as a simple sequence seed.
    seq = static_cast<int>(time(nullptr));
}

SocketCANManager::~SocketCANManager() {
    if (nlSocket) {
        mnl_socket_close(nlSocket);
    }
}

// Callback function to parse netlink messages.
// We filter messages to only include those whose ifi_type is ARPHRD_CAN.
int SocketCANManager::data_cb(const struct nlmsghdr *nlh, void *data) {
    std::vector<std::string>* interfaces = static_cast<std::vector<std::string>*>(data);
    struct ifinfomsg *ifm = reinterpret_cast<struct ifinfomsg*>(mnl_nlmsg_get_payload(nlh));
    
    // Only process CAN interfaces.
    if (ifm->ifi_type != ARPHRD_CAN)
        return MNL_CB_OK;

    // Loop over the attributes in the message.
    struct mnl_attr *attr = nullptr;
    mnl_attr_for_each(attr, nlh, sizeof(struct ifinfomsg)) {
        if (mnl_attr_get_type(attr) == IFLA_IFNAME) {
            const char* name = mnl_attr_get_str(attr);
            interfaces->push_back(std::string(name));
        }
    }
    return MNL_CB_OK;
}

std::vector<std::string> SocketCANManager::getSocketCANInterfaces() {
    std::vector<std::string> interfaces;
    char buf[MNL_SOCKET_BUFFER_SIZE];

    // Prepare the netlink message header.
    struct nlmsghdr *nlh = mnl_nlmsg_put_header(buf);
    nlh->nlmsg_type = RTM_GETLINK;
    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    nlh->nlmsg_seq = ++seq;

    // Append an ifinfomsg structure.
    struct ifinfomsg *ifm = reinterpret_cast<struct ifinfomsg*>(mnl_nlmsg_put_extra_header(nlh, sizeof(struct ifinfomsg)));
    ifm->ifi_family = AF_UNSPEC; // Query all families

    // Send the request.
    if (mnl_socket_sendto(nlSocket, nlh, nlh->nlmsg_len) < 0) {
        std::cerr << "Error: mnl_socket_sendto failed." << std::endl;
        return interfaces;
    }

    // Process the response messages.
    int ret = mnl_socket_recvfrom(nlSocket, buf, sizeof(buf));
    while (ret > 0) {
        ret = mnl_cb_run(buf, ret, nlh->nlmsg_seq, 0, data_cb, &interfaces);
        if (ret <= 0)
            break;
        ret = mnl_socket_recvfrom(nlSocket, buf, sizeof(buf));
    }
    return interfaces;
}

bool SocketCANManager::setInterfaceUp(const std::string& ifName) {
    int ifindex = if_nametoindex(ifName.c_str());
    if (ifindex == 0) {
        std::cerr << "Interface not found: " << ifName << std::endl;
        return false;
    }

    char buf[MNL_SOCKET_BUFFER_SIZE];
    struct nlmsghdr *nlh = mnl_nlmsg_put_header(buf);
    nlh->nlmsg_type = RTM_NEWLINK;
    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    nlh->nlmsg_seq = ++seq;

    // Set up the ifinfomsg message.
    struct ifinfomsg *ifm = reinterpret_cast<struct ifinfomsg*>(mnl_nlmsg_put_extra_header(nlh, sizeof(struct ifinfomsg)));
    ifm->ifi_family = AF_UNSPEC;
    ifm->ifi_index = ifindex;
    ifm->ifi_change = 0xffffffff; // update all flags

    // Bring the interface up by setting the IFF_UP flag.
    ifm->ifi_flags |= IFF_UP;

    // Send the message.
    if (mnl_socket_sendto(nlSocket, nlh, nlh->nlmsg_len) < 0) {
        std::cerr << "Error: mnl_socket_sendto failed in setInterfaceUp." << std::endl;
        return false;
    }

    // For simplicity, we read the ACK and assume success.
    mnl_socket_recvfrom(nlSocket, buf, sizeof(buf));
    return true;
}

bool SocketCANManager::setInterfaceDown(const std::string& ifName) {
    int ifindex = if_nametoindex(ifName.c_str());
    if (ifindex == 0) {
        std::cerr << "Interface not found: " << ifName << std::endl;
        return false;
    }

    char buf[MNL_SOCKET_BUFFER_SIZE];
    struct nlmsghdr *nlh = mnl_nlmsg_put_header(buf);
    nlh->nlmsg_type = RTM_NEWLINK;
    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    nlh->nlmsg_seq = ++seq;

    struct ifinfomsg *ifm = reinterpret_cast<struct ifinfomsg*>(mnl_nlmsg_put_extra_header(nlh, sizeof(struct ifinfomsg)));
    ifm->ifi_family = AF_UNSPEC;
    ifm->ifi_index = ifindex;
    ifm->ifi_change = 0xffffffff;

    // Bring the interface down by clearing the IFF_UP flag.
    ifm->ifi_flags &= ~IFF_UP;

    if (mnl_socket_sendto(nlSocket, nlh, nlh->nlmsg_len) < 0) {
        std::cerr << "Error: mnl_socket_sendto failed in setInterfaceDown." << std::endl;
        return false;
    }

    mnl_socket_recvfrom(nlSocket, buf, sizeof(buf));
    return true;
}

// Example usage of the SocketCANManager class.
// int usage_example() {
//     SocketCANManager manager;

//     // List available SocketCAN interfaces.
//     std::vector<std::string> canInterfaces = manager.getSocketCANInterfaces();
//     std::cout << "SocketCAN Interfaces found:" << std::endl;
//     for (const auto& iface : canInterfaces) {
//         std::cout << "  " << iface << std::endl;
//     }

//     // Example: bring the first interface up (if available).
//     if (!canInterfaces.empty()) {
//         std::cout << "Bringing " << canInterfaces[0] << " up..." << std::endl;
//         if (manager.setInterfaceUp(canInterfaces[0])) {
//             std::cout << "Interface " << canInterfaces[0] << " is up." << std::endl;
//         }
//         // Then bring it down.
//         std::cout << "Bringing " << canInterfaces[0] << " down..." << std::endl;
//         if (manager.setInterfaceDown(canInterfaces[0])) {
//             std::cout << "Interface " << canInterfaces[0] << " is down." << std::endl;
//         }
//     } else {
//         std::cout << "No SocketCAN interfaces detected." << std::endl;
//     }

//     return 0;
// }


#endif // LIBSOCKCANPP_INCLUDE_NETLINK_INTERFACEMANAGER_HPP