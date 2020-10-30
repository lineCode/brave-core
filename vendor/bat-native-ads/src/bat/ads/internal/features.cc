// Copyright (c) 2020 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#include "bat/ads/internal/features.h"

namespace ads {
namespace features {

// Controls behaviour of the contextual ad matching mechanism, e.g. by
// adjusting the number of page classifications used to infer user interest.
const base::Feature kContextualAdsControl{
    "ContextualAdsControl",
    base::FEATURE_DISABLED_BY_DEFAULT};

}  // namespace features
}  // namespace ads
