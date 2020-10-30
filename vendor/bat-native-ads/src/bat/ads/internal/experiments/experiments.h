/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BAT_ADS_INTERNAL_EXPERIMENTS_EXPERIMENTS_H_
#define BAT_ADS_INTERNAL_EXPERIMENTS_EXPERIMENTS_H_

#include <stdint.h>

#include <string>
#include <vector>

#include "base/values.h"

namespace ads {

struct ActiveTrialInfo {
  std::string trial_name;
  std::string group_name;
  std::string parameter_value;
};

namespace experiments {

bool HasActiveTrial();

uint32_t GetPageProbabilitiesHistorySize();

ActiveTrialInfo GetActiveTrialInfo();

}  // namespace experiments
}  // namespace ads

#endif  // BAT_ADS_INTERNAL_EXPERIMENTS_EXPERIMENTS_H_
