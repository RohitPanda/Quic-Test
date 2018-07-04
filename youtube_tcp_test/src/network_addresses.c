//
// Created by sergei on 18.06.18.
//

#include <ifaddrs.h>
#include <stdio.h>
#include <netinet/in.h>

#include "network_addresses.h"


enum ip_support get_ip_version_support()
{
    enum ip_support result_support = IP_SUPPORT_NONE;
    struct ifaddrs* local_adresses = NULL;
    struct ifaddrs * address = NULL;
    getifaddrs(&local_adresses);
    for (address = local_adresses; address != NULL; address = address->ifa_next) {
        if (address->ifa_addr->sa_family==AF_INET) { // Check it is
            // a valid IPv4 address
            struct sockaddr_in* addr = (struct sockaddr_in*)address->ifa_addr;
            if (addr->sin_addr.s_addr == INADDR_LOOPBACK)
                continue;
            if (result_support == IP_SUPPORT_NONE)
                result_support = IP_SUPPORT_IPV4;
            else if (result_support == IP_SUPPORT_IPV6)
            {
                result_support = IP_SUPPORT_BOTH;
                break;
            }

        }
        else if (address->ifa_addr->sa_family==AF_INET6) { // Check it is
            struct sockaddr_in6* addr = (struct sockaddr_in6*)address->ifa_addr;
            if (IN6_IS_ADDR_LOOPBACK(&addr->sin6_addr))
                continue;
            if (result_support == IP_SUPPORT_NONE)
                result_support = IP_SUPPORT_IPV6;
            else if (result_support == IP_SUPPORT_IPV4)
            {
                result_support = IP_SUPPORT_BOTH;
                break;
            }
        }
    }
    freeifaddrs(local_adresses);
    return result_support;
}