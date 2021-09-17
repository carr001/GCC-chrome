#include <assert.h>
#include "src/google_cc/receiver_loss_collection.h"
#include "src/common/defs.h"
namespace webrtc{


namespace{

template<typename T>
void SwapValue(T& a,T& b){
    T tmp;
    tmp = a;
    a = b;
    b = tmp;
}
const float kAlpha = 0.125f;
const float kOneMinusAlpha = (1 - kAlpha);
}// anonymous namespace

ReceiveVector::ReceiveVector(ReceiveVectorState s)
    :first_received_time(Timestamp::Micros(0)),
    first_packet_number(0),
    wait_duration(TimeDelta::Micros(0)),
    largest_packet_number(0),
    largest_received_time(Timestamp::Micros(0)),
    state(s)
    {}

void ReceiveVector::Reset(){
    received_vector.clear();

    first_received_time = Timestamp::Micros(0);
    wait_duration = TimeDelta::Micros(0);
    first_packet_number = 0;
    largest_packet_number =0;
    largest_received_time = Timestamp::Micros(0);
    state = ReceiveVectorState::IN_EFFECT;
}

ReceiverLossCollection::ReceiverLossCollection()
    :
    receive_duration_(TimeDelta::Millis(200)),
    smooth_rtt_(TimeDelta::Micros(0)),
    receive_vecotr1(ReceiveVector(ReceiveVectorState::IN_EFFECT)),
    receive_vecotr2(ReceiveVector(ReceiveVectorState::WAIT)),
    in_effect_vector_(&receive_vecotr1),
    wait_vector_(&receive_vecotr2)
    {}

LossReportUpdate ReceiverLossCollection::OnTransportPacketsFeedback(TransportPacketsFeedback msg){
    LossReportUpdate result;
    LossReportUpdate report;
    for(auto packet:msg.packet_feedbacks){
        report =  OnTransportPacketFeedback(packet);
        if(report.updated)
            result = report;
    }
    result = result.updated ? result : report;// return ethier the updated report or the last report
    return result;
}

LossReportUpdate ReceiverLossCollection::OnTransportPacketFeedback(PacketResult packet){
    LossReportUpdate result;

    if(packet.sent_packet.sequence_number<in_effect_vector_->first_packet_number){
        UpdateWaitVector(packet);
        result = MaybeGenerateReport(packet.receive_time);// must put after UpdateWaitVector
    }else{
        result = MaybeGenerateReport(packet.receive_time);// must put before UpdateEffectVector
        UpdateEffectVector(packet);
    }
    return result;
}
void ReceiverLossCollection::UpdateWaitVector(PacketResult packet){
    assert(packet.sent_packet.sequence_number<wait_vector_->largest_packet_number);
    uint64_t acked_indx = packet.sent_packet.sequence_number-wait_vector_->first_packet_number;
    // if(!wait_vector_->received_vector[acked_indx] ){
    wait_vector_->received_vector[acked_indx] = true;
    // }
    return ;
};
void ReceiverLossCollection::UpdateEffectVector(PacketResult packet){
    assert(packet.sent_packet.sequence_number>=in_effect_vector_->first_packet_number);
    if(packet.sent_packet.sequence_number>in_effect_vector_->largest_packet_number){
        if(in_effect_vector_->received_vector.empty()){
            in_effect_vector_->received_vector.push_back(true);
            in_effect_vector_->first_received_time = packet.receive_time;
            in_effect_vector_->largest_received_time = packet.receive_time;
            in_effect_vector_->first_packet_number = packet.sent_packet.sequence_number;
            in_effect_vector_->largest_packet_number = packet.sent_packet.sequence_number;
        }else{
            for(uint64_t gap = in_effect_vector_->largest_packet_number+1;gap<packet.sent_packet.sequence_number;gap++){
                in_effect_vector_->received_vector.push_back(false);
            }
            
            in_effect_vector_->received_vector.push_back(true);
            in_effect_vector_->largest_packet_number = packet.sent_packet.sequence_number;
            in_effect_vector_->largest_received_time = packet.receive_time;
        }
    }else{
        uint64_t acked_indx = packet.sent_packet.sequence_number-in_effect_vector_->first_packet_number;
        wait_vector_->received_vector[acked_indx] = true;
    }
    UpdateRtt(packet);
    if(packet.receive_time>in_effect_vector_->first_received_time+receive_duration_){
        OnReceiveDurationEnd();
    }
};

LossReportUpdate ReceiverLossCollection::MaybeGenerateReport(Timestamp end_time){
    LossReportUpdate report;
    report.updated = false;
    if(end_time>wait_vector_->first_received_time+receive_duration_+wait_vector_->wait_duration){
        uint64_t received_count=0;
        uint64_t loss_count=0;
        for(auto it=wait_vector_->received_vector.begin();it!=wait_vector_->received_vector.end();it++){
            if(*it == true)
                received_count++;
            else
                loss_count++;
        }
        if(received_count!=0){
            report.report.start_time = wait_vector_->first_received_time;
            report.report.end_time = end_time;
            report.report.packets_received_delta = received_count;
            report.report.packets_lost_delta = loss_count;

            OnNewReportGenerated();
            report.updated = true;
        }
    }
    return report;
}
TimeDelta ReceiverLossCollection::GetWaitDuration(){
    // wait duration mush not bigger than receive duration
    return MAX(MIN(smooth_rtt_,receive_duration_-TimeDelta::Millis(10)),TimeDelta::Millis(100));
}

void ReceiverLossCollection::OnReceiveDurationEnd(){
    // ReceiveVector* tmp;
    assert(wait_vector_->received_vector.empty());
    SwapValue(wait_vector_,in_effect_vector_); // change state
    wait_vector_->wait_duration = GetWaitDuration();
    smooth_rtt_ = TimeDelta::Micros(0);
    in_effect_vector_->first_packet_number = wait_vector_->largest_packet_number+1;
    // in_effect_vector_->Reset(); .. don't have to ,because we Reset OnNewReportGenerated
    assert(in_effect_vector_->received_vector.empty());
}

void ReceiverLossCollection::OnNewReportGenerated(){
    assert(in_effect_vector_->largest_received_time<in_effect_vector_->first_received_time+receive_duration_);
    wait_vector_->Reset();
}

void ReceiverLossCollection::UpdateRtt(PacketResult packet){
    TimeDelta rtt_sample = packet.receive_time-packet.sent_packet.send_time;
    if(smooth_rtt_==TimeDelta::Micros(0)){
        smooth_rtt_ = rtt_sample;
    }else{
        smooth_rtt_ = kOneMinusAlpha*smooth_rtt_+kAlpha*rtt_sample;
    }
}


}


