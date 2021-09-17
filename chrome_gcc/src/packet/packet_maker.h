#ifndef PACKET_MAKER_H
#define PACKET_MAKER_H 1

#include "src/packet/packet.h"
#include "src/quic/quic_time.h"
#include "src/time/timestamp.h"

class  GCCDataPacketMaker
{
public:
    GCCDataPacketMaker(/* args */);
    ~ GCCDataPacketMaker()=default;

    GCCDataPacket MakePacket();
    uint64_t GetRecentPacketNumber();

private:
    void ResetTemplate();

    int64_t recent_packet_number_;
    GCCDataPacket* packet_template_;
};


class  GCCRRPacketMaker
{
public:
    GCCRRPacketMaker(/* args */);
    ~ GCCRRPacketMaker()=default;

    GCCRRPacket MakePacket(webrtc::Timestamp /*start_time*/, 
        webrtc::Timestamp  /*end_time*/,
        uint64_t /*packets_lost_delta*/,
        uint64_t /*packets_received_delta*/);
private:
    void ResetTemplate();

    GCCRRPacket* packet_template_;
};

class  GCCREMBPacketMaker
{
public:
    GCCREMBPacketMaker(/* args */);
    ~ GCCREMBPacketMaker()=default;

    GCCREMBPacket MakePacket( webrtc::DataRate );
private:
    void ResetTemplate();

    GCCREMBPacket* packet_template_;
};




#endif
