/** @date 2015

    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

// Copyright 2015 Vuzix Corporation.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


/// Needed to get M_PI from math.h on Windows.
#define _USE_MATH_DEFINES

// Internal Includes
#include <osvr/PluginKit/PluginKit.h>
#include <osvr/PluginKit/EyeTrackerInterface.h>

// Generated JSON header file
#include "com_osvr_EyeTracker_json.h"

// Library/third-party includes
#include <math.h>

// Standard includes
#include <iostream>
#include <memory>

// Anonymous namespace to avoid symbol collision
namespace {

	//typedef std::shared_ptr<EyeTrackerInstance> TrackerPtr;

	OSVR_MessageType eyeTrackerMessage;

class EyeTrackerDevice {
  public:
	  EyeTrackerDevice(OSVR_PluginRegContext ctx) {
        /// Create the initialization options
        OSVR_DeviceInitOptions opts = osvrDeviceCreateInitOptions(ctx);

        //tracker = trackerInst;
		m_eyetracker = osvr::pluginkit::EyeTrackerInterface(opts);

        /// Create the sync device token with the options
        m_dev.initSync(ctx, "EyeTracker", opts);

        /// Send JSON descriptor
        m_dev.sendJsonDescriptor(com_osvr_EyeTracker_json);

        /// Register update callback
        m_dev.registerUpdateCallback(this);
    }

    OSVR_ReturnCode update() {

		OSVR_EyeGazeDirection gaze;
		OSVR_TimeValue times;

		osvrTimeValueGetNow(&times);

		OSVR_Vec2 gaze2D;
		gaze2D.data[0] = 0.5;
		gaze2D.data[1] = 0.5;

		OSVR_Vec3 gaze3D;
		gaze3D.data[0] = 1.5;
		gaze3D.data[1] = 1.5;
		gaze3D.data[2] = 1.5;

		gaze.gazeDirection2D = gaze2D;
		gaze.gazeDirection3D = gaze3D;


		m_dev.send(m_eyetracker, osvr::pluginkit::EyeTrackerMessage(gaze));
        
        return OSVR_RETURN_SUCCESS;
    }

  private:
    osvr::pluginkit::DeviceToken m_dev;
	osvr::pluginkit::EyeTrackerInterface m_eyetracker;
};

class HardwareDetection {
  public:
	  HardwareDetection() : m_found(false) {}
    OSVR_ReturnCode operator()(OSVR_PluginRegContext ctx) {

        std::cout << "PLUGIN: Got a hardware detection request" << std::endl;

		if (m_found) {
			return OSVR_RETURN_SUCCESS;
		}
        
		if (0 == 1){
			std::cout << "PLUGIN: We have NOT detected Eye Tracker "
                      << std::endl;
            return OSVR_RETURN_FAILURE;
        }

		m_found = true;
		
		std::cout << "PLUGIN: We have detected Eye Tracker device! " << std::endl;
		/// Create our device object
		osvr::pluginkit::registerObjectForDeletion(ctx, new EyeTrackerDevice(ctx));


        return OSVR_RETURN_SUCCESS;
    }

    //TrackerPtr tracker;


private:
	bool m_found;
};
} // namespace

OSVR_PLUGIN(com_osvr_EyeTracker) {

	osvrDeviceRegisterMessageType(ctx, "EyeTrackerMessage", &eyeTrackerMessage);

    osvr::pluginkit::PluginContext context(ctx);

    /// Register a detection callback function object.
    context.registerHardwareDetectCallback(new HardwareDetection());

    return OSVR_RETURN_SUCCESS;
}