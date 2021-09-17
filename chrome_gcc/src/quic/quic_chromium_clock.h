// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_QUIC_PLATFORM_IMPL_QUIC_CHROMIUM_CLOCK_H_
#define NET_QUIC_PLATFORM_IMPL_QUIC_CHROMIUM_CLOCK_H_


#include "src/quic/quic_clock.h"
#include "src/time/time.h"

// Clock to efficiently retrieve an approximately accurate time from an
// net::EpollServer.
class  QuicChromiumClock : public QuicClock {
 public:
  // static QuicChromiumClock* GetInstance();

  QuicChromiumClock();
  ~QuicChromiumClock() override;

  // QuicClock implementation:
  QuicTime ApproximateNow() const override;
  QuicTime Now() const override;
  // QuicWallTime WallNow() const override;

  // Converts a QuicTime returned by QuicChromiumClock to base::TimeTicks.
  // Helper functions to safely convert between QuicTime and TimeTicks.
  static base::TimeTicks QuicTimeToTimeTicks(QuicTime quic_time);

};


#endif  // NET_QUIC_PLATFORM_IMPL_QUIC_CHROMIUM_CLOCK_H_
