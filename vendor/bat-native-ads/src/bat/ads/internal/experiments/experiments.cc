/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/experiments/experiments.h"

#include <iostream>

#include "base/feature_list.h"
#include "base/metrics/field_trial.h"
#include "base/metrics/field_trial_params.h"
#include "base/strings/stringprintf.h"
#include "bat/ads/internal/features.h"

namespace ads {
namespace experiments {

namespace {

const char kActiveTrial[] = "PageProbabilitiesHistoryStudy";
const uint64_t kDefaultPageProbabilitiesHistoryEntries = 5;

}  // namespace

bool HasActiveTrial() {
  if (!base::FieldTrialList::Find(kActiveTrial)) {
    return false;
  }

  return true;
}

uint32_t GetPageProbabilitiesHistorySize() {
  return GetFieldTrialParamByFeatureAsInt(
      features::kContextualAdsControl,
      "page_probabilities_history_entries",
      kDefaultPageProbabilitiesHistoryEntries);
}

ActiveTrialInfo GetActiveTrialInfo() {
  ActiveTrialInfo active_trial_info;

  base::FieldTrial* trial = base::FieldTrialList::Find(kActiveTrial);
  if (!trial) {
    return active_trial_info;
  }

  active_trial_info = {
    trial->trial_name().c_str(),
    trial->group_name().c_str(),
    std::to_string(GetPageProbabilitiesHistorySize())
  };

  return active_trial_info;
}

}  // namespace experiments
}  // namespace ads
