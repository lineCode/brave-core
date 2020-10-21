/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/eligible_ads/ad_notifications/eligible_ad_notifications.h"

#include "bat/ads/internal/ads_impl.h"
#include "bat/ads/internal/client/client.h"
#include "bat/ads/internal/frequency_capping/ad_notifications/ad_notifications_frequency_capping.h"
#include "bat/ads/internal/logging.h"

namespace ads {
namespace ad_notifications {

namespace {

}  // namespace

EligibleAds::EligibleAds(
    AdsImpl* ads)
    : ads_(ads) {
  DCHECK(ads_);
}

EligibleAds::~EligibleAds() = default;

CreativeAdNotificationList EligibleAds::Get(
    const CreativeAdNotificationList& ads,
    const CreativeAdNotificationInfo& last_delivered_ad,
    const AdEventList& ad_events) {
  CreativeAdNotificationList eligible_ads;

  CreativeAdNotificationList unseen_ads =
      GetUnseenAdsAndRoundRobinIfNeeded(ads, last_delivered_ad);

  FrequencyCapping frequency_capping(ads_, ad_events);

  for (const auto& ad : unseen_ads) {
    if (frequency_capping.ShouldExcludeAd(ad)) {
      continue;
    }

    eligible_ads.push_back(ad);
  }

  return eligible_ads;
}

///////////////////////////////////////////////////////////////////////////////

CreativeAdNotificationList EligibleAds::GetUnseenAdsAndRoundRobinIfNeeded(
    const CreativeAdNotificationList& ads,
    const CreativeAdNotificationInfo& last_delivered_ad) const {
  if (ads.empty()) {
    return ads;
  }

  CreativeAdNotificationList ads_for_unseen_advertisers =
      GetAdsForUnseenAdvertisers(ads);
  if (ads_for_unseen_advertisers.empty()) {
    BLOG(1, "All advertisers have been shown, so round robin");

    const bool should_not_show_last_advertiser =
        ads_->get_client()->GetSeenAdvertisers().size() > 1 ? true : false;

    ads_->get_client()->ResetSeenAdvertisers(ads);

    ads_for_unseen_advertisers = GetAdsForUnseenAdvertisers(ads);

    if (should_not_show_last_advertiser) {
      const auto iter = std::remove_if(ads_for_unseen_advertisers.begin(),
          ads_for_unseen_advertisers.end(),
              [&](CreativeAdNotificationInfo& ad) {
        return ad.advertiser_id == last_delivered_ad.advertiser_id;
      });

      ads_for_unseen_advertisers.erase(iter, ads_for_unseen_advertisers.end());
    }
  }

  CreativeAdNotificationList unseen_ads =
      GetUnseenAds(ads_for_unseen_advertisers);
  if (unseen_ads.empty()) {
    BLOG(1, "All ads have been shown, so round robin");

    const bool should_not_show_last_ad =
        ads_->get_client()->GetSeenAdNotifications().size() > 1 ? true : false;

    ads_->get_client()->ResetSeenAdNotifications(ads);

    unseen_ads = GetUnseenAds(ads);

    if (should_not_show_last_ad) {
      const auto iter = std::remove_if(ads_for_unseen_advertisers.begin(),
          ads_for_unseen_advertisers.end(),
              [&](CreativeAdNotificationInfo& ad) {
        return ad.creative_instance_id ==
            last_delivered_ad.creative_instance_id;
      });

      ads_for_unseen_advertisers.erase(iter, ads_for_unseen_advertisers.end());
    }
  }

  return unseen_ads;
}

CreativeAdNotificationList EligibleAds::GetUnseenAds(
    const CreativeAdNotificationList& ads) const {
  CreativeAdNotificationList unseen_ads = ads;

  const auto seen_ads = ads_->get_client()->GetSeenAdNotifications();

  const auto seen_advertisers = ads_->get_client()->GetSeenAdvertisers();

  const auto iter = std::remove_if(unseen_ads.begin(), unseen_ads.end(),
      [&](CreativeAdNotificationInfo& ad) {
    return seen_ads.find(ad.creative_instance_id) != seen_ads.end() &&
        seen_ads.find(ad.advertiser_id) != seen_advertisers.end();
  });

  unseen_ads.erase(iter, unseen_ads.end());

  return unseen_ads;
}

CreativeAdNotificationList EligibleAds::GetAdsForUnseenAdvertisers(
    const CreativeAdNotificationList& ads) const {
  CreativeAdNotificationList unseen_ads = ads;

  const auto seen_ads = ads_->get_client()->GetSeenAdvertisers();

  const auto iter = std::remove_if(unseen_ads.begin(), unseen_ads.end(),
      [&seen_ads](CreativeAdNotificationInfo& ad) {
    return seen_ads.find(ad.advertiser_id) != seen_ads.end();
  });

  unseen_ads.erase(iter, unseen_ads.end());

  return unseen_ads;
}

}  // namespace ad_notifications
}  // namespace ads
