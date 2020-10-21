/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/ads_impl.h"

#include <utility>

#include "bat/ads/ad_history_info.h"
#include "bat/ads/ad_notification_info.h"
#include "bat/ads/ads_client.h"
#include "bat/ads/confirmation_type.h"
#include "bat/ads/internal/ad_rewards/ad_rewards.h"
#include "bat/ads/internal/ad_server/ad_server.h"
#include "bat/ads/internal/ad_serving/ad_notifications/ad_notification_serving.h"
#include "bat/ads/internal/ad_targeting/ad_targeting.h"
#include "bat/ads/internal/ad_targeting/behavioral/purchase_intent_classifier/purchase_intent_classifier.h"
#include "bat/ads/internal/ad_targeting/behavioral/purchase_intent_classifier/purchase_intent_classifier_user_models.h"
#include "bat/ads/internal/ad_targeting/contextual/page_classifier/page_classifier.h"
#include "bat/ads/internal/ad_targeting/contextual/page_classifier/page_classifier_user_models.h"
#include "bat/ads/internal/ad_targeting/geographic/subdivision/subdivision_targeting.h"
#include "bat/ads/internal/ad_transfer/ad_transfer.h"
#include "bat/ads/internal/ads/ad_notifications/ad_notification.h"
#include "bat/ads/internal/ads/ad_notifications/ad_notifications.h"
#include "bat/ads/internal/ads/new_tab_page_ads/new_tab_page_ad.h"
#include "bat/ads/internal/ads_history/ads_history.h"
#include "bat/ads/internal/bundle/bundle.h"
#include "bat/ads/internal/client/client.h"
#include "bat/ads/internal/confirmations/confirmation_info.h"
#include "bat/ads/internal/confirmations/confirmations.h"
#include "bat/ads/internal/conversions/conversions.h"
#include "bat/ads/internal/database/database_initialize.h"
#include "bat/ads/internal/logging.h"
#include "bat/ads/internal/platform/platform_helper.h"
#include "bat/ads/internal/privacy/unblinded_tokens/unblinded_tokens.h"
#include "bat/ads/internal/time_util.h"
#include "bat/ads/internal/tokens/redeem_unblinded_payment_tokens/redeem_unblinded_payment_tokens.h"
#include "bat/ads/internal/tokens/redeem_unblinded_token/redeem_unblinded_token.h"
#include "bat/ads/internal/tokens/refill_unblinded_tokens/refill_unblinded_tokens.h"
#include "bat/ads/internal/user_activity/user_activity.h"
#include "bat/ads/internal/wallet/wallet.h"
#include "bat/ads/pref_names.h"

#if defined(OS_ANDROID)
#include "base/android/build_info.h"
#include "base/system/sys_info.h"
#endif

namespace ads {

using std::placeholders::_1;

namespace {
const int kIdleThresholdInSeconds = 15;
}  // namespace

AdsImpl::AdsImpl(
    AdsClient* ads_client)
    : ads_client_(ads_client),
      ads_history_(std::make_unique<AdsHistory>(this)),
      ad_notification_(std::make_unique<AdNotification>(this)),
      ad_notifications_(std::make_unique<AdNotifications>(this)),
      ad_rewards_(std::make_unique<AdRewards>(this)),
      ad_server_(std::make_unique<AdServer>(this)),
      ad_notification_serving_(std::make_unique<
          ad_notifications::AdServing>(this)),
      ad_targeting_(std::make_unique<AdTargeting>(this)),
      ad_transfer_(std::make_unique<AdTransfer>(this)),
      bundle_(std::make_unique<Bundle>(this)),
      client_(std::make_unique<Client>(this)),
      confirmations_(std::make_unique<Confirmations>(this)),
      conversions_(std::make_unique<Conversions>(this)),
      database_(std::make_unique<database::Initialize>(this)),
      new_tab_page_ad_(std::make_unique<NewTabPageAd>(this)),
      page_classifier_(std::make_unique<
          ad_targeting::contextual::PageClassifier>(this)),
      purchase_intent_classifier_(std::make_unique<
          ad_targeting::behavioral::PurchaseIntentClassifier>(this)),
      redeem_unblinded_payment_tokens_(std::make_unique<
          RedeemUnblindedPaymentTokens>(this)),
      redeem_unblinded_token_(std::make_unique<RedeemUnblindedToken>(this)),
      refill_unblinded_tokens_(std::make_unique<RefillUnblindedTokens>(this)),
      subdivision_targeting_(std::make_unique<
          ad_targeting::geographic::SubdivisionTargeting>(this)),
      user_activity_(std::make_unique<UserActivity>()),
      wallet_(std::make_unique<Wallet>(this)) {
  set_ads_client_for_logging(ads_client_);

  redeem_unblinded_token_->set_delegate(this);
  redeem_unblinded_payment_tokens_->set_delegate(this);
  refill_unblinded_tokens_->set_delegate(this);
}

AdsImpl::~AdsImpl() = default;

void AdsImpl::Initialize(
    InitializeCallback callback) {
  BLOG(1, "Initializing ads");

  if (IsInitialized()) {
    BLOG(1, "Already initialized ads");
    callback(FAILED);
    return;
  }

  const auto initialize_step_2_callback =
      std::bind(&AdsImpl::InitializeStep2, this, _1, std::move(callback));
  database_->CreateOrOpen(initialize_step_2_callback);
}

void AdsImpl::InitializeStep2(
    const Result result,
    InitializeCallback callback) {
  if (result != SUCCESS) {
    BLOG(0, "Failed to initialize database: " << database_->get_last_message());
    callback(FAILED);
    return;
  }

  const auto initialize_step_3_callback =
      std::bind(&AdsImpl::InitializeStep3, this, _1, std::move(callback));
  client_->Initialize(initialize_step_3_callback);
}

void AdsImpl::InitializeStep3(
    const Result result,
    InitializeCallback callback) {
  if (result != SUCCESS) {
    callback(FAILED);
    return;
  }

  const auto initialize_step_4_callback =
      std::bind(&AdsImpl::InitializeStep4, this, _1, std::move(callback));
  confirmations_->Initialize(initialize_step_4_callback);
}

void AdsImpl::InitializeStep4(
    const Result result,
    InitializeCallback callback) {
  if (result != SUCCESS) {
    callback(FAILED);
    return;
  }

  const auto initialize_step_5_callback =
      std::bind(&AdsImpl::InitializeStep5, this, _1, std::move(callback));
  ad_notifications_->Initialize(initialize_step_5_callback);
}

void AdsImpl::InitializeStep5(
    const Result result,
    InitializeCallback callback) {
  if (result != SUCCESS) {
    callback(FAILED);
    return;
  }

  const auto initialize_step_6_callback =
      std::bind(&AdsImpl::InitializeStep6, this, _1, std::move(callback));
  conversions_->Initialize(initialize_step_6_callback);
}

void AdsImpl::InitializeStep6(
    const Result result,
    InitializeCallback callback) {
  if (result != SUCCESS) {
    callback(FAILED);
    return;
  }

  is_initialized_ = true;

  BLOG(1, "Successfully initialized ads");

  is_foreground_ = ads_client_->IsForeground();

  ads_client_->SetIntegerPref(prefs::kIdleThreshold, kIdleThresholdInSeconds);

  callback(SUCCESS);

  ReconcileAdRewards();

  subdivision_targeting_->MaybeFetchForCurrentLocale();

  const WalletInfo wallet = wallet_->Get();
  redeem_unblinded_payment_tokens_->MaybeRedeemAfterDelay(wallet);

  conversions_->StartTimerIfReady();

#if defined(OS_ANDROID)
    // Ad notifications do not sustain a reboot or update, so we should remove
    // orphaned ad notifications
    RemoveAllAdNotificationsAfterReboot();
    RemoveAllAdNotificationsAfterUpdate();
#endif

  MaybeServeAdNotificationsAtRegularIntervals();

  ad_server_->MaybeFetch();
}

#if defined(OS_ANDROID)
void AdsImpl::RemoveAllAdNotificationsAfterReboot() {
  auto ads_shown_history = ads_history_->Get();
  if (!ads_shown_history.empty()) {
    uint64_t ad_shown_timestamp =
        ads_shown_history.front().timestamp_in_seconds;
    uint64_t boot_timestamp =
        static_cast<uint64_t>(base::Time::Now().ToDoubleT() -
            static_cast<uint64_t>(base::SysInfo::Uptime().InSeconds()));
    if (ad_shown_timestamp <= boot_timestamp) {
      ad_notifications_->RemoveAll(false);
    }
  }
}

void AdsImpl::RemoveAllAdNotificationsAfterUpdate() {
  // Ad notifications do not sustain app update, so remove all ad notifications
  std::string current_version_code(
      base::android::BuildInfo::GetInstance()->package_version_code());
  std::string last_version_code = client_->GetVersionCode();
  if (last_version_code != current_version_code) {
    client_->SetVersionCode(current_version_code);
    ad_notifications_->RemoveAll(false);
  }
}
#endif

bool AdsImpl::IsInitialized() {
  if (!is_initialized_ || !ads_client_->GetBooleanPref(prefs::kEnabled)) {
    return false;
  }

  return true;
}

void AdsImpl::Shutdown(
    ShutdownCallback callback) {
  if (!is_initialized_) {
    BLOG(0, "Shutdown failed as not initialized");

    callback(FAILED);
    return;
  }

  ad_notifications_->RemoveAll(true);

  callback(SUCCESS);
}

bool AdsImpl::GetAdNotification(
    const std::string& uuid,
    AdNotificationInfo* notification) {
  return ad_notifications_->Get(uuid, notification);
}

void AdsImpl::OnForeground() {
  is_foreground_ = true;

  BLOG(1, "Browser window did become active");

  user_activity_->RecordActivityForType(
      UserActivityType::kBrowserWindowDidBecomeActive);

  MaybeServeAdNotificationsAtRegularIntervals();
}

void AdsImpl::OnBackground() {
  is_foreground_ = false;

  BLOG(1, "Browser window did enter background");

  user_activity_->RecordActivityForType(
      UserActivityType::kBrowserWindowDidEnterBackground);

  MaybeServeAdNotificationsAtRegularIntervals();
}

bool AdsImpl::IsForeground() const {
  return is_foreground_;
}

void AdsImpl::OnIdle() {
  BLOG(1, "Browser state changed to idle");
}

void AdsImpl::OnUnIdle() {
  if (!IsInitialized()) {
    BLOG(0, "OnUnIdle failed as not initialized");
    return;
  }

  BLOG(1, "Browser state changed to unidle");

  MaybeServeAdNotification();
}

void AdsImpl::MaybeServeAdNotification() {
  if (PlatformHelper::GetInstance()->IsMobile()) {
    return;
  }

  ad_notification_serving_->MaybeServe();
}

void AdsImpl::MaybeServeAdNotificationsAtRegularIntervals() {
  if (!PlatformHelper::GetInstance()->IsMobile()) {
    return;
  }

  if (is_foreground_ || ads_client_->CanShowBackgroundNotifications()) {
    ad_notification_serving_->ServeAtRegularIntervals();
  } else {
    ad_notification_serving_->StopServing();
  }
}

void AdsImpl::OnMediaPlaying(
    const int32_t tab_id) {
  const auto iter = media_playing_.find(tab_id);
  if (iter != media_playing_.end()) {
    // Media is already playing for this tab
    return;
  }

  BLOG(2, "Started playing media for tab id " << tab_id);

  media_playing_.insert(tab_id);

  user_activity_->RecordActivityForType(UserActivityType::kStartedPlayingMedia);
}

void AdsImpl::OnMediaStopped(
    const int32_t tab_id) {
  const auto iter = media_playing_.find(tab_id);
  if (iter == media_playing_.end()) {
    // Media is not playing for this tab
    return;
  }

  BLOG(2, "Stopped playing media for tab id " << tab_id);

  media_playing_.erase(iter);
}

bool AdsImpl::IsMediaPlaying() const {
  const auto iter = media_playing_.find(active_tab_id_);
  if (iter == media_playing_.end()) {
    // Media is not playing in the active tab
    return false;
  }

  return true;
}

void AdsImpl::OnAdNotificationEvent(
    const std::string& uuid,
    const AdNotificationEventType event_type) {
  ad_notification_->Trigger(uuid, event_type);
}

void AdsImpl::OnNewTabPageAdEvent(
    const std::string& wallpaper_id,
    const std::string& creative_instance_id,
    const NewTabPageAdEventType event_type) {
  new_tab_page_ad_->Trigger(wallpaper_id, creative_instance_id, event_type);
}

int32_t AdsImpl::get_active_tab_id() const {
  return active_tab_id_;
}

std::string AdsImpl::get_active_tab_url() const {
  return active_tab_url_;
}

std::string AdsImpl::get_previous_tab_url() const {
  return previous_tab_url_;
}

void AdsImpl::OnTabUpdated(
    const int32_t tab_id,
    const std::string& url,
    const bool is_active,
    const bool is_browser_active,
    const bool is_incognito) {
  if (is_incognito) {
    return;
  }

  if (is_active && is_browser_active) {
    if (active_tab_id_ != tab_id) {
      BLOG(2, "Tab id " << tab_id << " is visible");

      user_activity_->RecordActivityForType(
          UserActivityType::kOpenedNewOrFocusedOnExistingTab);

      active_tab_id_ = tab_id;
      previous_tab_url_ = active_tab_url_;
      active_tab_url_ = url;
    }
  } else {
    BLOG(7, "Tab id " << tab_id << " is occluded");
  }
}

void AdsImpl::OnTabClosed(
    const int32_t tab_id) {
  BLOG(2, "Tab id " << tab_id << " was closed");

  OnMediaStopped(tab_id);

  ad_transfer_->Cancel(tab_id);

  user_activity_->RecordActivityForType(UserActivityType::kClosedTab);
}

void AdsImpl::OnWalletUpdated(
    const std::string& id,
    const std::string& seed) {
  if (!wallet_->Set(id, seed)) {
    BLOG(0, "Failed to update wallet");
    return;
  }

  BLOG(1, "Successfully updated wallet");
}

void AdsImpl::RemoveAllHistory(
    RemoveAllHistoryCallback callback) {
  client_->RemoveAllHistory();

  callback(SUCCESS);
}

AdsHistoryInfo AdsImpl::GetAdsHistory(
    const AdsHistoryInfo::FilterType filter_type,
    const AdsHistoryInfo::SortType sort_type,
    const uint64_t from_timestamp,
    const uint64_t to_timestamp) {
  return ads_history_->Get(filter_type, sort_type,
      from_timestamp, to_timestamp);
}

AdContentInfo::LikeAction AdsImpl::ToggleAdThumbUp(
    const std::string& creative_instance_id,
    const std::string& creative_set_id,
    const AdContentInfo::LikeAction& action) {
  auto like_action =
      client_->ToggleAdThumbUp(creative_instance_id, creative_set_id, action);
  if (like_action == AdContentInfo::LikeAction::kThumbsUp) {
    confirmations_->ConfirmAd(creative_instance_id, ConfirmationType::kUpvoted);
  }

  return like_action;
}

AdContentInfo::LikeAction AdsImpl::ToggleAdThumbDown(
    const std::string& creative_instance_id,
    const std::string& creative_set_id,
    const AdContentInfo::LikeAction& action) {
  auto like_action =
      client_->ToggleAdThumbDown(creative_instance_id, creative_set_id, action);
  if (like_action == AdContentInfo::LikeAction::kThumbsDown) {
    confirmations_->ConfirmAd(creative_instance_id,
        ConfirmationType::kDownvoted);
  }

  return like_action;
}

CategoryContentInfo::OptAction AdsImpl::ToggleAdOptInAction(
    const std::string& category,
    const CategoryContentInfo::OptAction& action) {
  return client_->ToggleAdOptInAction(category, action);
}

CategoryContentInfo::OptAction AdsImpl::ToggleAdOptOutAction(
    const std::string& category,
    const CategoryContentInfo::OptAction& action) {
  return client_->ToggleAdOptOutAction(category, action);
}

bool AdsImpl::ToggleSaveAd(
    const std::string& creative_instance_id,
    const std::string& creative_set_id,
    const bool saved) {
  return client_->ToggleSaveAd(creative_instance_id, creative_set_id, saved);
}

bool AdsImpl::ToggleFlagAd(
    const std::string& creative_instance_id,
    const std::string& creative_set_id,
    const bool flagged) {
  auto flag_ad =
      client_->ToggleFlagAd(creative_instance_id, creative_set_id, flagged);
  if (flag_ad) {
    confirmations_->ConfirmAd(creative_instance_id, ConfirmationType::kFlagged);
  }

  return flag_ad;
}

void AdsImpl::ChangeLocale(
    const std::string& locale) {
  subdivision_targeting_->MaybeFetchForLocale(locale);
  page_classifier_->LoadUserModelForLocale(locale);
  purchase_intent_classifier_->LoadUserModelForLocale(locale);
}

void AdsImpl::OnUserModelUpdated(
    const std::string& id) {
  if (kPageClassificationUserModelIds.find(id) !=
      kPageClassificationUserModelIds.end()) {
    page_classifier_->LoadUserModelForId(id);
  } else if (kPurchaseIntentUserModelIds.find(id) !=
      kPurchaseIntentUserModelIds.end()) {
    purchase_intent_classifier_->LoadUserModelForId(id);
  } else {
    BLOG(0, "Unknown " << id << " user model");
  }
}

void AdsImpl::OnAdsSubdivisionTargetingCodeHasChanged() {
  subdivision_targeting_->MaybeFetchForCurrentLocale();
}

void AdsImpl::OnPageLoaded(
    const int32_t tab_id,
    const std::string& original_url,
    const std::string& url,
    const std::string& content) {
  DCHECK(!original_url.empty());
  DCHECK(!url.empty());

  if (!IsInitialized()) {
    BLOG(1, "OnPageLoaded failed as not initialized");
    return;
  }

  ad_transfer_->MaybeTransferAd(tab_id, original_url);

  conversions_->MaybeConvert(url);

  purchase_intent_classifier_->MaybeExtractIntentSignal(url);

  page_classifier_->MaybeClassifyPage(url, content);
}

///////////////////////////////////////////////////////////////////////////////

void AdsImpl::ReconcileAdRewards() {
  if (!IsInitialized()) {
    return;
  }

  const WalletInfo wallet = wallet_->Get();
  ad_rewards_->MaybeReconcile(wallet);
}

void AdsImpl::GetTransactionHistory(
    GetTransactionHistoryCallback callback) {
  StatementInfo statement;

  if (!IsInitialized()) {
    callback(/* success */ false, statement);
    return;
  }

  statement.estimated_pending_rewards =
      get_ad_rewards()->GetEstimatedPendingRewards();

  statement.next_payment_date_in_seconds =
      get_ad_rewards()->GetNextPaymentDateInSeconds();

  statement.ad_notifications_received_this_month =
      get_ad_rewards()->GetAdNotificationsReceivedThisMonth();

  auto to_timestamp_in_seconds =
      static_cast<uint64_t>(base::Time::Now().ToDoubleT());
  const TransactionList transactions =
      GetTransactions(0, to_timestamp_in_seconds);
  statement.transactions = transactions;

  callback(/* success */ true, statement);
}

TransactionList AdsImpl::GetTransactions(
    const uint64_t from_timestamp_in_seconds,
    const uint64_t to_timestamp_in_seconds) {
  TransactionList transactions = confirmations_->get_transactions();

  const auto iter = std::remove_if(transactions.begin(), transactions.end(),
      [&](TransactionInfo& transaction) {
    return transaction.timestamp_in_seconds < from_timestamp_in_seconds ||
        transaction.timestamp_in_seconds > to_timestamp_in_seconds;
  });

  transactions.erase(iter, transactions.end());

  return transactions;
}

TransactionList AdsImpl::GetUnredeemedTransactions() {
  const size_t count = confirmations_->get_unblinded_payment_tokens()->Count();
  if (count == 0) {
    // There are no outstanding unblinded payment tokens to redeem
    return {};
  }

  // Unredeemed transactions are always at the end of the transaction history
  const TransactionList transactions = confirmations_->get_transactions();
  if (transactions.size() < count) {
    // There are fewer transactions than unblinded payment tokens which is
    // likely due to manually editing transactions in confirmations.json
    NOTREACHED();
    return transactions;
  }

  const TransactionList tail_transactions(transactions.end() - count,
      transactions.end());

  return tail_transactions;
}

void AdsImpl::OnDidRedeemUnblindedToken(
    const ConfirmationInfo& confirmation) {
  BLOG(1, "Successfully redeemed unblinded token with confirmation id "
      << confirmation.id << ", creative instance id "
          << confirmation.creative_instance_id << " and "
              << std::string(confirmation.type));
}

void AdsImpl::OnFailedToRedeemUnblindedToken(
    const ConfirmationInfo& confirmation) {
  BLOG(1, "Failed to redeem unblinded token with confirmation id "
      << confirmation.id << ", creative instance id "
          <<  confirmation.creative_instance_id << " and "
              << std::string(confirmation.type));
}

void AdsImpl::OnDidRedeemUnblindedPaymentTokens() {
  BLOG(1, "Successfully redeemed unblinded payment tokens");

  ReconcileAdRewards();
}

void AdsImpl::OnFailedToRedeemUnblindedPaymentTokens() {
  BLOG(1, "Failed to redeem unblinded payment tokens");
}

void AdsImpl::OnDidRetryRedeemingUnblindedPaymentTokens() {
  BLOG(1, "Retry redeeming unblinded payment tokens");
}

void AdsImpl::OnDidRefillUnblindedTokens() {
  BLOG(1, "Successfully refilled unblinded tokens");
}

void AdsImpl::OnFailedToRefillUnblindedTokens() {
  BLOG(1, "Failed to refill unblinded tokens");
}

void AdsImpl::OnDidRetryRefillingUnblindedTokens() {
  BLOG(1, "Retry refilling unblinded tokens");
}

}  // namespace ads
