// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <limits>

#include "src/quic/quic_clock.h"
#include "src/quic/quic_time.h"


QuicClock::QuicClock()
    : is_calibrated_(false), calibration_offset_(QuicTime::Delta::Zero()) {}

QuicClock::~QuicClock() {}

void QuicClock::SetCalibrationOffset(QuicTime::Delta offset) {
  calibration_offset_ = offset;
  is_calibrated_ = true;
}



