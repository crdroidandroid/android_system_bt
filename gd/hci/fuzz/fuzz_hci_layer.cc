/*
 * Copyright 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "hci/fuzz/fuzz_hci_layer.h"
#include "fuzz/helpers.h"

namespace bluetooth {
namespace hci {
namespace fuzz {

using bluetooth::common::ContextualCallback;
using bluetooth::fuzz::GetArbitraryBytes;

common::BidiQueueEnd<hci::AclPacketBuilder, hci::AclPacketView>* FuzzHciLayer::GetAclQueueEnd() {
  return acl_queue_.GetUpEnd();
}

hci::SecurityInterface* FuzzHciLayer::GetSecurityInterface(
    ContextualCallback<void(hci::EventPacketView)> event_handler) {
  return &security_interface_;
}

hci::LeSecurityInterface* FuzzHciLayer::GetLeSecurityInterface(
    ContextualCallback<void(hci::LeMetaEventView)> event_handler) {
  return &le_security_interface_;
}

hci::AclConnectionInterface* FuzzHciLayer::GetAclConnectionInterface(
    ContextualCallback<void(hci::EventPacketView)> event_handler,
    ContextualCallback<void(uint16_t, hci::ErrorCode)> on_disconnect) {
  return &acl_connection_interface_;
}

hci::LeAclConnectionInterface* FuzzHciLayer::GetLeAclConnectionInterface(
    ContextualCallback<void(hci::LeMetaEventView)> event_handler,
    ContextualCallback<void(uint16_t, hci::ErrorCode)> on_disconnect) {
  return &le_acl_connection_interface_;
}

hci::LeAdvertisingInterface* FuzzHciLayer::GetLeAdvertisingInterface(
    ContextualCallback<void(hci::LeMetaEventView)> event_handler) {
  return &le_advertising_interface_;
}

hci::LeScanningInterface* FuzzHciLayer::GetLeScanningInterface(
    ContextualCallback<void(hci::LeMetaEventView)> event_handler) {
  return &le_scanning_interface_;
}

void FuzzHciLayer::Start() {
  acl_dev_null_ = new os::fuzz::DevNullQueue<AclPacketBuilder>(acl_queue_.GetDownEnd(), GetHandler());
  acl_dev_null_->Start();
  acl_inject_ = new os::fuzz::FuzzInjectQueue<AclPacketView>(acl_queue_.GetDownEnd(), GetHandler());
}

void FuzzHciLayer::Stop() {
  acl_dev_null_->Stop();
  delete acl_dev_null_;
  delete acl_inject_;
}

void FuzzHciLayer::injectArbitrary(FuzzedDataProvider& fdp) {
  const uint8_t action = fdp.ConsumeIntegralInRange(0, 1);
  switch (action) {
    case 1:
      injectAclData(GetArbitraryBytes(&fdp));
      break;
  }
}

void FuzzHciLayer::injectAclData(std::vector<uint8_t> data) {
  hci::AclPacketView aclPacket = hci::AclPacketView::FromBytes(data);
  if (!aclPacket.IsValid()) {
    return;
  }

  acl_inject_->Inject(std::make_unique<AclPacketView>(aclPacket));
}

const ModuleFactory FuzzHciLayer::Factory = ModuleFactory([]() { return new FuzzHciLayer(); });

}  // namespace fuzz
}  // namespace hci
}  // namespace bluetooth