/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <string>

#define BRAVE_VERIFY_SEED_SIGNATURE \
  return VerifySignatureResult::VALID_SIGNATURE;

#include "../../../../components/variations/variations_seed_store.cc"
#undef BRAVE_VERIFY_SEED_SIGNATURE
