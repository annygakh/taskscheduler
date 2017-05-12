//
// Created by Anaid Gakhokidze on 2017-05-12.
//

#ifndef TASKSCHEDULER_PING_H
#define TASKSCHEDULER_PING_H

#include <list>
namespace Ping
{

// ICMP request packet structure
typedef struct {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t identifier;
    uint16_t sequence_number;
    uint32_t data;
} IcmpRequestPktStruct;

// Checksum method based on in_chksum()
// from Unit Network Programming by Stevens, Fenner and Rudoff
int32_t checksum(uint16_t *buf, int32_t len);

std::unordered_map<std::string, double> ping(void);

}

#endif //TASKSCHEDULER_PING_H
