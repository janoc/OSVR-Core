/** @file
    @brief Implementation

    @date 2015

    @author
    Sensics, Inc.
    <http://sensics.com/osvr>

*/

// Copyright 2015 Sensics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// 	http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Internal Includes
#include <osvr/PluginKit/EyeTrackerInterfaceC.h>
#include <osvr/Connection/DeviceInitObject.h>
//#include <osvr/Connection/ImagingServerInterface.h>
#include <osvr/Connection/DeviceToken.h>
#include <osvr/PluginHost/PluginSpecificRegistrationContext.h>
#include <osvr/Common/EyeTrackerComponent.h>
#include "PointerWrapper.h"
#include "HandleNullContext.h"
#include <osvr/Util/Verbosity.h>

// Library/third-party includes
// - none

// Standard includes
// - none

struct OSVR_EyeTrackerDeviceInterfaceObject {
	osvr::common::EyeTrackerComponent *eyetracker;
};

OSVR_ReturnCode
osvrDeviceEyeTrackerConfigure(OSVR_INOUT_PTR OSVR_DeviceInitOptions opts,
OSVR_OUT_PTR OSVR_EyeTrackerDeviceInterface *iface,
OSVR_IN OSVR_ChannelCount numSensors) {

	OSVR_PLUGIN_HANDLE_NULL_CONTEXT("osvrDeviceEyeTrackerConfigure", opts);
	OSVR_PLUGIN_HANDLE_NULL_CONTEXT("osvrDeviceEyeTrackerConfigure", iface);
	OSVR_EyeTrackerDeviceInterface ifaceObj =
		opts->getContext()->registerDataWithGenericDelete(
		new OSVR_EyeTrackerDeviceInterfaceObject);
	*iface = ifaceObj;
	auto eyeTracker = osvr::common::EyeTrackerComponent::create(numSensors);
	ifaceObj->eyetracker = eyeTracker.get();
	opts->addComponent(eyeTracker);
	return OSVR_RETURN_SUCCESS;
}

OSVR_ReturnCode
osvrDeviceEyeTrackerReportData(OSVR_IN_PTR OSVR_DeviceToken dev,
OSVR_IN_PTR OSVR_EyeTrackerDeviceInterface iface,
OSVR_IN_PTR OSVR_EyeGazeDirection eyeData,
OSVR_IN OSVR_ChannelCount sensor,
OSVR_IN_PTR OSVR_TimeValue const *timestamp) {
	auto guard = dev->getSendGuard();
	if (guard->lock()) {
		iface->eyetracker->sendEyeData(eyeData, sensor, *timestamp);
		return OSVR_RETURN_SUCCESS;
	}

	return OSVR_RETURN_FAILURE;
}