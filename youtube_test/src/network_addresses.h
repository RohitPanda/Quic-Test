//
// Created by sergei on 18.06.18.
//

#ifndef YOUTUBE_TEST_NETWORK_ADDRESSES_H
#define YOUTUBE_TEST_NETWORK_ADDRESSES_H

/*
 * returns 0 on none, 1 on
 */

enum ip_support{
    IP_SUPPORT_NONE, IP_SUPPORT_IPV4, IP_SUPPORT_IPV6, IP_SUPPORT_BOTH
};
enum ip_support get_ip_version_support();

#endif //YOUTUBE_TEST_NETWORK_ADDRESSES_H
