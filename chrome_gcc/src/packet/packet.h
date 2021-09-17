#ifndef PACKET_H
#define PACKET_H 

#include <stddef.h>
#include <vector>
#include <string>
#include "src/common/defs.h"
#include "src/common/data_rate.h"
#include "src/time/timestamp.h"

class Packet
{
public:
    Packet(/* args */);
    // ~Packet();
    virtual const std::string   Serialize()=0;
};

// send time
class GCCDataPacket:public Packet
{
public:
    GCCDataPacket(/* args */);
    // ~SerializedPacket();
    const std::string  Serialize() override;
    
    PACKET_TYPE type=PACKET_TYPE::DATA;
    uint64_t sequence_number;
    uint64_t size=DEFAULT_PACKET_SIZE;
    webrtc::Timestamp send_time;
};

class GCCRRPacket:public Packet
{
public:
    GCCRRPacket(/* args */);
    // ~SerializedPacket();
    const std::string  Serialize() override;

    PACKET_TYPE type=PACKET_TYPE::RR;
    webrtc::Timestamp start_time;
    webrtc::Timestamp end_time;
    uint64_t packets_lost_delta;
    uint64_t packets_received_delta;
};

class GCCREMBPacket:public Packet
{
public:
    GCCREMBPacket(/* args */);
    // ~SerializedPacket();
    const std::string  Serialize() override;

    PACKET_TYPE type=PACKET_TYPE::REMB;
    webrtc::DataRate bandwidth;
};



#endif