#ifndef RECEIVER_LOSS_COLLECTION_H
#define RECEIVER_LOSS_COLLECTION_H

#include "src/common/network_types.h"
#include "src/time/timestamp.h"

#include <vector>

namespace webrtc{

struct LossReportUpdate{
    TransportLossReport report;
    bool updated=false;
};
enum ReceiveVectorState{
    IN_EFFECT,
    WAIT,
};

class ReceiveVector
{
public:
    ReceiveVector(ReceiveVectorState s);
    void Reset();
    Timestamp first_received_time;
    Timestamp largest_received_time;
    TimeDelta wait_duration;
    uint64_t first_packet_number;
    uint64_t largest_packet_number;
    std::vector<bool> received_vector;
    ReceiveVectorState state;
};


class ReceiverLossCollection
{
public:
    ReceiverLossCollection(/* args */);
    ~ReceiverLossCollection();

    LossReportUpdate OnTransportPacketsFeedback(TransportPacketsFeedback msg);
    LossReportUpdate OnTransportPacketFeedback(PacketResult packet);

private:
    void UpdateWaitVector(PacketResult packet);
    void UpdateEffectVector(PacketResult packet);
    void OnReceiveDurationEnd();
    void OnNewReportGenerated();
    void UpdateRtt(PacketResult packet);
    LossReportUpdate MaybeGenerateReport(Timestamp /*end_time*/);
    TimeDelta GetWaitDuration();

    const TimeDelta receive_duration_;
    TimeDelta smooth_rtt_;

    TransportLossReport latest_report_;
    
    ReceiveVector* in_effect_vector_;
    ReceiveVector* wait_vector_;

    ReceiveVector receive_vecotr1;
    ReceiveVector receive_vecotr2;

};



}


#endif