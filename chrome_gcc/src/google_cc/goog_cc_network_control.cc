/*
 *  Copyright (c) 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "src/google_cc/goog_cc_network_control.h"

#include <inttypes.h>
#include <stdio.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include "src/time/time_delta.h"
#include "src/google_cc/bwe_defines.h"

namespace webrtc {

namespace {
// From RTCPSender video report interval.
constexpr TimeDelta kLossUpdateInterval = TimeDelta::Millis(1000);

// Pacing-rate relative to our target send rate.
// Multiplicative factor that is applied to the target bitrate to calculate
// the number of bytes that can be transmitted per interval.
// Increasing this factor will result in lower delays in cases of bitrate
// overshoots from the encoder.
constexpr float kDefaultPaceMultiplier = 2.5f;

// If the probe result is far below the current throughput estimate
// it's unlikely that the probe is accurate, so we don't want to drop too far.
// However, if we actually are overusing, we want to drop to something slightly
// below the current throughput estimate to drain the network queues.
constexpr double kProbeDropThroughputFraction = 0.85;

int64_t GetBpsOrDefault(const absl::optional<DataRate>& rate,
                        int64_t fallback_bps) {
  if (rate && rate->IsFinite()) {
    return rate->bps();
  } else {
    return fallback_bps;
  }
}

// bool IsEnabled(const WebRtcKeyValueConfig* config, absl::string_view key) {
//   return absl::StartsWith(config->Lookup(key), "Enabled");
// }

// bool IsNotDisabled(const WebRtcKeyValueConfig* config, absl::string_view key) {
//   return !absl::StartsWith(config->Lookup(key), "Disabled");
// }
}  // namespace

GoogCcNetworkController::GoogCcNetworkController(NetworkControllerConfig config)
    : packet_feedback_only_(false),
      bandwidth_estimation_(
          std::make_unique<SendSideBandwidthEstimation>()),
      delay_based_bwe_(new DelayBasedBwe()),
      acknowledged_bitrate_estimator_(
          AcknowledgedBitrateEstimatorInterface::Create()),
      initial_config_(config),
      last_loss_based_target_rate_(*config.constraints.starting_rate),
      last_pushback_target_rate_(last_loss_based_target_rate_),
      last_stable_target_rate_(last_loss_based_target_rate_),
      pacing_factor_(config.stream_based_config.pacing_factor.value_or(
          kDefaultPaceMultiplier)),
      min_total_allocated_bitrate_(
          config.stream_based_config.min_total_allocated_bitrate.value_or(
              DataRate::Zero())),
      max_padding_rate_(config.stream_based_config.max_padding_rate.value_or(
          DataRate::Zero())),
      max_total_allocated_bitrate_(DataRate::Zero()) {
  if (delay_based_bwe_)
    delay_based_bwe_->SetMinBitrate(congestion_controller::GetMinBitrate());
}

GoogCcNetworkController::~GoogCcNetworkController() {}

NetworkControlUpdate GoogCcNetworkController::OnNetworkAvailability(
    NetworkAvailability msg) {
  NetworkControlUpdate update;
  return update;
}

NetworkControlUpdate GoogCcNetworkController::OnNetworkRouteChange(
    NetworkRouteChange msg) {
  NetworkControlUpdate update;
  return update;
}

NetworkControlUpdate GoogCcNetworkController::OnProcessInterval(
    ProcessInterval msg) {
  NetworkControlUpdate update;
  return update;
}

NetworkControlUpdate GoogCcNetworkController::OnRemoteBitrateReport(
    RemoteBitrateReport msg) {
  if (packet_feedback_only_) {
    return NetworkControlUpdate();
  }
  bandwidth_estimation_->UpdateReceiverEstimate(msg.receive_time,
                                                msg.bandwidth);
  return NetworkControlUpdate();
}

NetworkControlUpdate GoogCcNetworkController::OnRoundTripTimeUpdate(
    RoundTripTimeUpdate msg) {
  if (packet_feedback_only_ || msg.smoothed)
    return NetworkControlUpdate();
  if (delay_based_bwe_)
    delay_based_bwe_->OnRttUpdate(msg.round_trip_time);
  bandwidth_estimation_->UpdateRtt(msg.round_trip_time, msg.receive_time);
  return NetworkControlUpdate();
}

NetworkControlUpdate GoogCcNetworkController::OnSentPacket(
    SentPacket sent_packet) {
  if (!first_packet_sent_) {
    first_packet_sent_ = true;
    // Initialize feedback time to send time to allow estimation of RTT until
    // first feedback is received.
    bandwidth_estimation_->UpdatePropagationRtt(sent_packet.send_time,
                                                TimeDelta::Zero());
  }
  bandwidth_estimation_->OnSentPacket(sent_packet);

  return NetworkControlUpdate();
}

NetworkControlUpdate GoogCcNetworkController::OnReceivedPacket(
    ReceivedPacket received_packet) {
  last_packet_received_time_ = received_packet.receive_time;
  return NetworkControlUpdate();
}

NetworkControlUpdate GoogCcNetworkController::OnStreamsConfig(
    StreamsConfig msg) {
  NetworkControlUpdate update;
  return update;
}

NetworkControlUpdate GoogCcNetworkController::OnTargetRateConstraints(
    TargetRateConstraints constraints) {
  NetworkControlUpdate update;
  update.probe_cluster_configs = ResetConstraints(constraints);
  MaybeTriggerOnNetworkChanged(&update, constraints.at_time);
  return update;
}

void GoogCcNetworkController::ClampConstraints() {
  // TODO(holmer): We should make sure the default bitrates are set to 10 kbps,
  // and that we don't try to set the min bitrate to 0 from any applications.
  // The congestion controller should allow a min bitrate of 0.
  min_data_rate_ =
      std::max(min_target_rate_, congestion_controller::GetMinBitrate());
  if (max_data_rate_ < min_data_rate_) {
    max_data_rate_ = min_data_rate_;
  }
  if (starting_rate_ && starting_rate_ < min_data_rate_) {
    starting_rate_ = min_data_rate_;
  }
}

std::vector<ProbeClusterConfig> GoogCcNetworkController::ResetConstraints(
    TargetRateConstraints new_constraints) {
  min_target_rate_ = new_constraints.min_data_rate.value_or(DataRate::Zero());
  max_data_rate_ =
      new_constraints.max_data_rate.value_or(DataRate::PlusInfinity());
  starting_rate_ = new_constraints.starting_rate;
  ClampConstraints();

  bandwidth_estimation_->SetBitrates(starting_rate_, min_data_rate_,
                                     max_data_rate_, new_constraints.at_time);

  if (starting_rate_)
    delay_based_bwe_->SetStartBitrate(*starting_rate_);
  delay_based_bwe_->SetMinBitrate(min_data_rate_);
  return std::vector<ProbeClusterConfig> ();
}

NetworkControlUpdate GoogCcNetworkController::OnTransportLossReport(
    TransportLossReport msg) {
  if (packet_feedback_only_)
    return NetworkControlUpdate();
  int64_t total_packets_delta =
      msg.packets_received_delta + msg.packets_lost_delta;
  bandwidth_estimation_->UpdatePacketsLost(
      msg.packets_lost_delta, total_packets_delta, msg.receive_time);
  return NetworkControlUpdate();
}

void GoogCcNetworkController::UpdateCongestionWindowSize() {

}

NetworkControlUpdate GoogCcNetworkController::OnTransportPacketsFeedback(
    TransportPacketsFeedback report) {
  if (report.packet_feedbacks.empty()) {
    // TODO(bugs.webrtc.org/10125): Design a better mechanism to safe-guard
    // against building very large network queues.
    return NetworkControlUpdate();
  }
  TimeDelta max_feedback_rtt = TimeDelta::MinusInfinity();
  TimeDelta min_propagation_rtt = TimeDelta::PlusInfinity();
  Timestamp max_recv_time = Timestamp::MinusInfinity();

  std::vector<PacketResult> feedbacks = report.ReceivedWithSendInfo();
  for (const auto& feedback : feedbacks)
    max_recv_time = std::max(max_recv_time, feedback.receive_time);

  for (const auto& feedback : feedbacks) {
    TimeDelta feedback_rtt =
        report.feedback_time - feedback.sent_packet.send_time;
    TimeDelta min_pending_time = feedback.receive_time - max_recv_time;
    TimeDelta propagation_rtt = feedback_rtt - min_pending_time;
    max_feedback_rtt = std::max(max_feedback_rtt, feedback_rtt);
    min_propagation_rtt = std::min(min_propagation_rtt, propagation_rtt);
  }

  if (max_feedback_rtt.IsFinite()) {
    feedback_max_rtts_.push_back(max_feedback_rtt.ms());
    const size_t kMaxFeedbackRttWindow = 32;
    if (feedback_max_rtts_.size() > kMaxFeedbackRttWindow)
      feedback_max_rtts_.pop_front();
    // TODO(srte): Use time since last unacknowledged packet.
    bandwidth_estimation_->UpdatePropagationRtt(report.feedback_time,
                                                min_propagation_rtt);
  }
  if (packet_feedback_only_) {
    if (!feedback_max_rtts_.empty()) {
      int64_t sum_rtt_ms = std::accumulate(feedback_max_rtts_.begin(),
                                           feedback_max_rtts_.end(), 0);
      int64_t mean_rtt_ms = sum_rtt_ms / feedback_max_rtts_.size();
      if (delay_based_bwe_)
        delay_based_bwe_->OnRttUpdate(TimeDelta::Millis(mean_rtt_ms));
    }

    TimeDelta feedback_min_rtt = TimeDelta::PlusInfinity();
    for (const auto& packet_feedback : feedbacks) {
      TimeDelta pending_time = packet_feedback.receive_time - max_recv_time;
      TimeDelta rtt = report.feedback_time -
                      packet_feedback.sent_packet.send_time - pending_time;
      // Value used for predicting NACK round trip time in FEC controller.
      feedback_min_rtt = std::min(rtt, feedback_min_rtt);
    }
    if (feedback_min_rtt.IsFinite()) {
      bandwidth_estimation_->UpdateRtt(feedback_min_rtt, report.feedback_time);
    }

    expected_packets_since_last_loss_update_ +=
        report.PacketsWithFeedback().size();
    for (const auto& packet_feedback : report.PacketsWithFeedback()) {
      if (!packet_feedback.IsReceived())
        lost_packets_since_last_loss_update_ += 1;
    }
    if (report.feedback_time > next_loss_update_) {
      next_loss_update_ = report.feedback_time + kLossUpdateInterval;
      bandwidth_estimation_->UpdatePacketsLost(
          lost_packets_since_last_loss_update_,
          expected_packets_since_last_loss_update_, report.feedback_time);
      expected_packets_since_last_loss_update_ = 0;
      lost_packets_since_last_loss_update_ = 0;
    }
  }

  acknowledged_bitrate_estimator_->IncomingPacketFeedbackVector(
      report.SortedByReceiveTime());
  auto acknowledged_bitrate = acknowledged_bitrate_estimator_->bitrate();
  bandwidth_estimation_->SetAcknowledgedRate(acknowledged_bitrate,
                                             report.feedback_time);
  bandwidth_estimation_->IncomingPacketFeedbackVector(report);

  NetworkControlUpdate update;
  bool recovered_from_overuse = false;
  bool backoff_in_alr = false;

  DelayBasedBwe::Result result;
  result = delay_based_bwe_->IncomingPacketFeedbackVector(
      report, acknowledged_bitrate, estimate_,
      false);

  recent_result_ = result;

  return update;
}

NetworkControlUpdate GoogCcNetworkController::OnNetworkStateEstimate(
    NetworkStateEstimate msg) {
  estimate_ = msg;
  return NetworkControlUpdate();
}

NetworkControlUpdate GoogCcNetworkController::GetNetworkState(
    Timestamp at_time) const {
  NetworkControlUpdate update;
  update.target_rate = TargetTransferRate();
  update.target_rate->network_estimate.at_time = at_time;
  update.target_rate->network_estimate.loss_rate_ratio =
      last_estimated_fraction_loss_.value_or(0) / 255.0;
  update.target_rate->network_estimate.round_trip_time =
      last_estimated_round_trip_time_;
  update.target_rate->network_estimate.bwe_period =
      delay_based_bwe_->GetExpectedBwePeriod();

  update.target_rate->at_time = at_time;
  update.target_rate->target_rate = last_pushback_target_rate_;
  update.target_rate->stable_target_rate =
      bandwidth_estimation_->GetEstimatedLinkCapacity();
  update.pacer_config = GetPacingRates(at_time);
  update.congestion_window = current_data_window_;
  return update;
}

void GoogCcNetworkController::MaybeTriggerOnNetworkChanged(
    NetworkControlUpdate* update,
    Timestamp at_time) {
  uint8_t fraction_loss = bandwidth_estimation_->fraction_loss();
  TimeDelta round_trip_time = bandwidth_estimation_->round_trip_time();
  DataRate loss_based_target_rate = bandwidth_estimation_->target_rate();
  DataRate pushback_target_rate = loss_based_target_rate;

  double cwnd_reduce_ratio = 0.0;

  DataRate stable_target_rate =
      bandwidth_estimation_->GetEstimatedLinkCapacity();

  stable_target_rate = std::min(stable_target_rate, pushback_target_rate);

  if ((loss_based_target_rate != last_loss_based_target_rate_) ||
      (fraction_loss != last_estimated_fraction_loss_) ||
      (round_trip_time != last_estimated_round_trip_time_) ||
      (pushback_target_rate != last_pushback_target_rate_) ||
      (stable_target_rate != last_stable_target_rate_)) {
    last_loss_based_target_rate_ = loss_based_target_rate;
    last_pushback_target_rate_ = pushback_target_rate;
    last_estimated_fraction_loss_ = fraction_loss;
    last_estimated_round_trip_time_ = round_trip_time;
    last_stable_target_rate_ = stable_target_rate;

    TimeDelta bwe_period = delay_based_bwe_->GetExpectedBwePeriod();

    TargetTransferRate target_rate_msg;
    target_rate_msg.at_time = at_time;

    target_rate_msg.target_rate = pushback_target_rate;
    target_rate_msg.stable_target_rate = stable_target_rate;
    target_rate_msg.network_estimate.at_time = at_time;
    target_rate_msg.network_estimate.round_trip_time = round_trip_time;
    target_rate_msg.network_estimate.loss_rate_ratio = fraction_loss / 255.0f;
    target_rate_msg.network_estimate.bwe_period = bwe_period;

    update->target_rate = target_rate_msg;

    update->pacer_config = GetPacingRates(at_time);
  }
}

PacerConfig GoogCcNetworkController::GetPacingRates(Timestamp at_time) const {
  // Pacing rate is based on target rate before congestion window pushback,
  // because we don't want to build queues in the pacer when pushback occurs.
  DataRate pacing_rate =
      std::max(min_total_allocated_bitrate_, last_loss_based_target_rate_) *
      pacing_factor_;
  DataRate padding_rate =
      std::min(max_padding_rate_, last_pushback_target_rate_);
  PacerConfig msg;
  msg.at_time = at_time;
  msg.time_window = TimeDelta::Seconds(1);
  msg.data_window = pacing_rate * msg.time_window;
  msg.pad_window = padding_rate * msg.time_window;
  return msg;
}

DataRate GoogCcNetworkController::GetSenderSideDataRate()const{
  return bandwidth_estimation_->target_rate();
}
DataRate GoogCcNetworkController::GetReceiverLimit()const{
  return bandwidth_estimation_->receiver_limit();
}


}  // namespace webrtc
