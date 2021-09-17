/*
 *  Copyright (c) 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "src/time/time_delta.h"

#include <string>

namespace webrtc {

std::string ToString(TimeDelta value) {
  std::string sb("");
  if (value.IsPlusInfinity()) {
    sb += "+inf ms";
  } else if (value.IsMinusInfinity()) {
    sb += "-inf ms";
  } else {
    if (value.us() == 0 || (value.us() % 1000) != 0)
      sb += std::to_string(value.us()) += " us";
    else if (value.ms() % 1000 != 0)
      sb += std::to_string(value.ms()) += " ms";
    else
      sb += std::to_string(value.seconds()) += " s";
  }
  return sb ;
}

}  // namespace webrtc
