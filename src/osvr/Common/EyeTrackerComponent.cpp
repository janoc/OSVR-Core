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
#include <osvr/Common/EyeTrackerComponent.h>
#include <osvr/Common/BaseDevice.h>
#include <osvr/Common/Serialization.h>
#include <osvr/Common/Buffer.h>
#include <osvr/Util/Verbosity.h>

// Library/third-party includes


namespace osvr {
	namespace common {

		namespace messages {
			class EyeRegion::MessageSerialization {
			public:
				MessageSerialization(OSVR_EyeGazeDirection const &gaze,
					OSVR_ChannelCount sensor)
					: m_data(gaze),
					m_sensor(sensor) {}

				MessageSerialization() {}

				template <typename T> void processMessage(T &p) {
					p(m_data.gazeDirection2D);
					p(m_data.gazeDirection3D);
				}
				EyeData getData() const {
					EyeData ret;
					ret.sensor = m_sensor;
					ret.gaze = m_data;
					return ret;
				}

			private:
				OSVR_EyeGazeDirection m_data;
				OSVR_ChannelCount m_sensor;
			};
			const char *EyeRegion::identifier() {
				return "com.osvr.eyetracker.eyeregion";
			}
		} // namespace messages

		shared_ptr<EyeTrackerComponent>
			EyeTrackerComponent::create(OSVR_ChannelCount numChan){
			shared_ptr<EyeTrackerComponent> ret(new EyeTrackerComponent(numChan));
			return ret;
		}

		EyeTrackerComponent::EyeTrackerComponent(OSVR_ChannelCount numChan)
			: m_numSensor(numChan) {}

		void EyeTrackerComponent::sendEyeData(OSVR_EyeGazeDirection data,
												OSVR_ChannelCount sensor,
												OSVR_TimeValue const &timestamp){
			
			Buffer<> buf;
			messages::EyeRegion::MessageSerialization msg(data, sensor);
			serialize(buf, msg);

			m_getParent().packMessage(buf, eyeRegion.getMessageType(), timestamp);
		}

		int VRPN_CALLBACK
			EyeTrackerComponent::m_handleEyeRegion(void *userdata, vrpn_HANDLERPARAM p) {
			auto self = static_cast<EyeTrackerComponent *>(userdata);
			auto bufwrap = ExternalBufferReadingWrapper<unsigned char>(
				reinterpret_cast<unsigned char const *>(p.buffer), p.payload_len);
			auto bufReader = BufferReader<decltype(bufwrap)>(bufwrap);

			messages::EyeRegion::MessageSerialization msg;
			//deserialize(bufReader, msg);
			auto data = msg.getData();
			auto timestamp = util::time::fromStructTimeval(p.msg_time);

			for (auto const &cb : self->m_cb) {
				cb(data, timestamp);
			}
			return 0;
		}

		void EyeTrackerComponent::registerEyeHandler(EyeHandler handler) {
			if (m_cb.empty()) {
				m_registerHandler(&EyeTrackerComponent::m_handleEyeRegion, this,
					eyeRegion.getMessageType());
			}
			m_cb.push_back(handler);
		}
		void EyeTrackerComponent::m_parentSet() {
			m_getParent().registerMessageType(eyeRegion);
		}

	} // namespace common
} // namespace osvr