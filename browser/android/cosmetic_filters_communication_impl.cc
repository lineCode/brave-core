/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 3.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/android/cosmetic_filters_communication_impl.h"

#include "base/json/json_reader.h"
#include "brave/browser/brave_browser_process_impl.h"
#include "brave/components/brave_shields/browser/ad_block_custom_filters_service.h"
#include "brave/components/brave_shields/browser/ad_block_regional_service_manager.h"
#include "brave/components/brave_shields/browser/ad_block_service.h"
#include "content/public/browser/render_frame_host.h"

CosmeticFiltersCommunicationImpl::CosmeticFiltersCommunicationImpl(
    content::RenderFrameHost* render_frame_host)
    : render_frame_host_(render_frame_host) {
}

CosmeticFiltersCommunicationImpl::~CosmeticFiltersCommunicationImpl() {
}

std::unique_ptr<base::ListValue>
    CosmeticFiltersCommunicationImpl::GetHiddenClassIdSelectorsOnTaskRunner(
        const std::vector<std::string>& classes,
        const std::vector<std::string>& ids) {
  // base::Optional<base::Value> hide_selectors = g_brave_browser_process->
  //     ad_block_service()->HiddenClassIdSelectors(classes, ids,
  //     	  render_frame_host_->cf_exceptions);

  // base::Optional<base::Value> regional_selectors = g_brave_browser_process->
  //     ad_block_regional_service_manager()->
  //         HiddenClassIdSelectors(classes, ids,
  //         	  render_frame_host_->cf_exceptions);

  // base::Optional<base::Value> custom_selectors = g_brave_browser_process->
  //     ad_block_custom_filters_service()->
  //         HiddenClassIdSelectors(classes, ids,
  //         	  render_frame_host_->cf_exceptions);

  // if (hide_selectors && hide_selectors->is_list()) {
  //   if (regional_selectors && regional_selectors->is_list()) {
  //     for (auto i = regional_selectors->GetList().begin();
  //         i < regional_selectors->GetList().end(); i++) {
  //       hide_selectors->Append(std::move(*i));
  //     }
  //   }
  // } else {
  //   hide_selectors = std::move(regional_selectors);
  // }

  auto result_list = std::make_unique<base::ListValue>();
  // if (hide_selectors && hide_selectors->is_list()) {
  //   result_list->Append(std::move(*hide_selectors));
  // }
  // if (custom_selectors && custom_selectors->is_list()) {
  //   result_list->Append(std::move(*custom_selectors));
  // }

  return result_list;
 }
 
 void CosmeticFiltersCommunicationImpl::GetHiddenClassIdSelectorsOnUI(
  	 std::unique_ptr<base::ListValue> selectors) {

 }

void CosmeticFiltersCommunicationImpl::HiddenClassIdSelectors(
	const std::string& input) {
  base::Optional<base::Value> input_value = base::JSONReader::Read(input);
  if (!input_value || !input_value->is_dict()) {
  	// Nothing to work with
  	return;
  }
  base::DictionaryValue* input_dict;
  if (!input_value->GetAsDictionary(&input_dict)) {
  	return;
  }
  std::vector<std::string> classes;
  base::ListValue* classes_list;
  if (input_dict->GetList("classes", &classes_list)) {
    for (size_t i = 0; i < classes_list->GetSize(); i++) {
      classes.push_back(classes_list->GetList()[i].GetString());
    }
  }
  std::vector<std::string> ids;
  base::ListValue* ids_list;
  if (input_dict->GetList("ids", &ids_list)) {
    for (size_t i = 0; i < ids_list->GetSize(); i++) {
      ids.push_back(ids_list->GetList()[i].GetString());
    }
  }

  // g_brave_browser_process->ad_block_service()->GetTaskRunner()->
  //     PostTaskAndReplyWithResult(FROM_HERE,
  //         base::BindOnce(&CosmeticFiltersCommunicationImpl::
  //           GetHiddenClassIdSelectorsOnTaskRunner, base::Unretained(this),
  //           classes, ids),
  //         base::BindOnce(&CosmeticFiltersCommunicationImpl::
  //           GetHiddenClassIdSelectorsOnUI, base::Unretained(this)));




  // for (size_t i = 0; i < render_frame_host_->cf_exceptions.size(); i++) {
  //   LOG(ERROR) << "!!!exception == " << render_frame_host_->cf_exceptions[i];
  // }

  // if (render_frame_host_->enabled_1st_party_cf_filtering) {
  //   // TODO
  //   // resources.force_hide_selectors.push(...resources.hide_selectors)
  //   force_hide_selectors_list = hide_selectors_list;
  // } else {
  // }

  // std::string cosmeticFilterConsiderNewSelectors_script =
  //         "(function() {"
  //           "let nextIndex = window.cosmeticStyleSheet.rules.length;";
}
