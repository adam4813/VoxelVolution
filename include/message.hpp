//
// chat_message.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef CHAT_MESSAGE_HPP
#define CHAT_MESSAGE_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include "message-types.hpp"
#include "component-update-system.hpp"

struct MessageHeader {
	static unsigned char SIGNATURE[]; 
	vv::frame_id_t frame_id;
	vv::message::message_type_t message_type;
	std::uint16_t message_size;
};

unsigned char MessageHeader::SIGNATURE[] = {'T', 'X', 'P', '1'};

class chat_message {
public:
	enum { header_length = 8 };
	enum { max_body_length = 512 };

	chat_message()
		: body_length_(0) { }

	const char* data() const {
		return data_;
	}

	char* data() {
		return data_;
	}

	std::size_t MessageLength() const {
		return sizeof(MessageHeader) + body_length_;
	}
	
	static std::size_t HeaderLength() {
		return sizeof(MessageHeader);
	}

	const char* body() const {
		return data_ + sizeof(MessageHeader);
	}

	char* body() {
		return data_ + sizeof(MessageHeader);
	}

	std::size_t body_length() const {
		return body_length_;
	}

	void body_length(std::size_t new_length) {
		body_length_ = new_length;
		if (body_length_ > max_body_length)
			body_length_ = max_body_length;
	}

	virtual bool decode_header() {
		MessageHeader msg_head;
		std::string str(data_);
		size_t offset = 4;
		msg_head.frame_id = atoi(str.substr(offset, sizeof(msg_head.frame_id)).c_str());
		offset += sizeof(msg_head.frame_id);
		msg_head.message_type = atoi(str.substr(offset, sizeof(msg_head.message_type)).c_str());
		offset += sizeof(msg_head.message_type);
		msg_head.message_size = body_length_ = atoi(str.substr(offset, sizeof(msg_head.message_size)).c_str());
		if (body_length_ > max_body_length) {
			body_length_ = 0;
			return false;
		}
		return true;
	}

	virtual void encode_header() {
		MessageHeader msg_head;
		msg_head.frame_id = 0;
		msg_head.message_type = vv::message::POSITION_CHANGE;
		msg_head.message_size = body_length_;
		vv::frame_id_t frame_id = 12;
		std::stringstream ss;
		ss << msg_head.SIGNATURE
			<< std::setfill('0') << std::setw(sizeof(msg_head.frame_id)) << msg_head.frame_id
			<< std::setfill('0') << std::setw(sizeof(msg_head.message_type)) << msg_head.message_type
			<< std::setfill('0') << std::setw(sizeof(msg_head.message_size)) << msg_head.message_size;
		std::memcpy(data_, ss.str().c_str(), ss.str().size());
	}

protected:
	char data_[header_length + max_body_length];
	std::size_t body_length_;
};

#endif // CHAT_MESSAGE_HPP
