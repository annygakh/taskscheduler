//
// Created by Anaid Gakhokidze on 2017-05-12.
//

#include "Ping.h"

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <cstring>
#include <string>

std::unordered_map<std::string, double> Ping::ping(void)
{
    bool succ = true, sockNeedToClose = false;
    std::chrono::steady_clock::time_point start, end;

    std::string host = "google.com";

    int sockfd = 0, rc;
    struct addrinfo hints, * servinfo;
    memset(&hints, 0, sizeof(hints));

    while (true)
    {
        IcmpRequestPktStruct requestPkt;
        memset(&requestPkt, 0, sizeof(requestPkt));

        requestPkt.type = 8;
        requestPkt.code = 0;
        requestPkt.checksum = 0;
        requestPkt.identifier = getpid();
        requestPkt.sequence_number = 1;
        requestPkt.data = 0;

        requestPkt.checksum = checksum((uint16_t *) &requestPkt, sizeof(requestPkt));


        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_RAW;
        hints.ai_protocol = IPPROTO_ICMP;
        start = std::chrono::steady_clock::now();

        if ((rc = getaddrinfo(host.c_str(), 0, &hints, &servinfo)) != 0)
        {
            succ = false;
            break;
        }

        sockNeedToClose = true;

        sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
        if (sockfd <= 0)
        {
            succ = false;
            break;
        }

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100000;
        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
            succ = false;
            break;
        }

        // send ping
        rc = 0;

        rc = sendto(sockfd, &requestPkt, sizeof(requestPkt), 0, servinfo->ai_addr, servinfo->ai_addrlen);

        if (rc < 0)
        {
            succ = false;
            break;
        }

        unsigned int respAddrSize;
        unsigned char resp[30];
        struct sockaddr respAddr;
        rc = 0;
        rc = recvfrom(sockfd, resp, sizeof(resp), 0, &respAddr,
                      &respAddrSize);

        if (rc <= 0)
        {
            // didnt receive a response
            succ = false;
        }
        break;
    }
    double timeElapsed;
    if (succ)
    {
        end = std::chrono::steady_clock::now();
        timeElapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    }
    else
    {
        timeElapsed = 0; // this way if the timeElapsed is 0, we know that the ping failed
    }
    std::unordered_map<std::string, double> metrics;
    metrics.insert({"timeElapsed", timeElapsed});

    freeaddrinfo(servinfo);
    if (sockNeedToClose) close(sockfd);
    return metrics;
}

int32_t Ping::checksum(uint16_t *buf, int32_t len)
{
    int32_t nleft = len;
    int32_t sum = 0;
    uint16_t *w = buf;
    uint16_t answer = 0;

    while (nleft > 1)
    {
        sum += *w++;
        nleft -= 2;
    }

    //  handle odd headers
    if (nleft == 1)
    {
        sum += *(uint8_t *)w;
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);

    answer = ~sum;

    return answer;
}