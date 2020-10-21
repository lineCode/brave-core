/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSInteger, BATAdsConfirmationType) {
  BATAdsCUndefinedonfirmationType,  // = ads::ConfirmationType::kUndefined
  BATAdsClickedConfirmationType,    // = ads::ConfirmationType::kClicked
  BATAdsDismissedConfirmationType,  // = ads::ConfirmationType::kDismissed
  BATAdsViewedConfirmationType,     // = ads::ConfirmationType::kViewed
  BATAdsLandedConfirmationType,     // = ads::ConfirmationType::kLanded
  BATAdsFlaggedConfirmationType,    // = ads::ConfirmationType::kFlagged
  BATAdsUpvotedConfirmationType,    // = ads::ConfirmationType::kUpvoted
  BATAdsDownvotedConfirmationType,  // = ads::ConfirmationType::kDownvoted
  BATAdsConversionConfirmationType  // = ads::ConfirmationType::kConversion
} NS_SWIFT_NAME(ConfirmationType);

NS_SWIFT_NAME(AdsNotification)
@interface BATAdNotification : NSObject
@property (nonatomic, readonly, copy) NSString *uuid;
@property (nonatomic, readonly, copy) NSString *creativeInstanceID;
@property (nonatomic, readonly, copy) NSString *creativeSetID;
@property (nonatomic, readonly, copy) NSString *campaignID;
@property (nonatomic, readonly, copy) NSString *category;
@property (nonatomic, readonly, copy) NSString *title;
@property (nonatomic, readonly, copy) NSString *body;
@property (nonatomic, readonly, copy) NSString *targetURL;
@end

@interface BATAdNotification (MyFirstAd)
+ (instancetype)customAdWithTitle:(NSString *)title
                             body:(NSString *)body
                              url:(NSString *)url NS_SWIFT_NAME(customAd(title:body:url:));
@end

NS_ASSUME_NONNULL_END
