/*
 *  Copyright (c) 2019 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include "src/common/frequency.h"

namespace webrtc {
  
std::string ToString(Frequency value) {
  std::string sb("");
  if (value.IsPlusInfinity()) {
    sb += "+inf Hz";
  } else if (value.IsMinusInfinity()) {
    sb += "-inf Hz";
  } else if (value.millihertz<int64_t>() % 1000 != 0) {
    sb += std::to_string(value.hertz<double>()) += "Hz";
  } else {
    sb += std::to_string(value.hertz<int64_t>())  += " Hz";
  }
  return sb;
}

}  // namespace webrtc
