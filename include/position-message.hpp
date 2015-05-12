#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "message.hpp"
#include "component-update-system.hpp"
#include "message-types.hpp"
#include <sstream>
#include <iomanip>

namespace vv {
	struct MessageHeader {
		unsigned char sig[4];
		frame_id_t frame_id;
		message::message_type_t message_type;
		unsigned short message_size;
	};

	class position_message : public chat_message {
	public:
		position_message() { }

		void FrameID(long long id) {
			this->frame_id = id;
		}

		bool decode_header() {
			std::string str(data_);
			body_length_ = atoi(str.substr(0, 4).c_str());
			msg_id = atoi(str.substr(4, 4).c_str());
			if (body_length_ > max_body_length) {
				body_length_ = 0;
				return false;
			}
			return true;
		}

		void encode_header() {
			std::stringstream ss;
			ss << std::setfill('0') << std::setw(4) << static_cast<int>(body_length_) <<
				std::setfill('0') << std::setw(4) << message::POSITION_CHANGE <<
				this->frame_id;
			std::memcpy(data_, ss.str().c_str(), ss.str().size());
		}

	private:
		long long frame_id;
		int msg_id;
	};
}
