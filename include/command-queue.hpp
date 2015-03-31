#pragma once

#include <queue>
#include <memory>
#include <atomic>
#include "multiton.hpp"

namespace vv {
	// Base class used for commands. T is the command id type (e.g. enum, int, char).
	template <typename T>
	struct Command {
		Command(T c, GUID entity_id) : command(c), entity_id(entity_id) { }
		T command;
		GUID entity_id;
	};

	// T is the command id type (e.g. enum, int, char), and U is the derived command type.
	template <class T>
	class CommandQueue {
	public:
		CommandQueue() :
			local_queue(new std::queue<std::shared_ptr<Command<T>>>()) { }
		~CommandQueue() { }

		template <typename U, typename V>
		static void QueueCommand(const T c, const GUID entity_id, V data = nullptr) {
			auto command = std::make_shared<U>(c, entity_id, data);
			(*global_queue).push(command);
		}

		static void QueueCommand(const T c, const GUID entity_id) {
			auto command = std::make_shared<Command<T>>(c, entity_id);
			(*global_queue).push(command);
		}
	protected:
		static std::atomic<std::queue<std::shared_ptr<Command<T>>>*> global_queue;
		std::queue<std::shared_ptr<Command<T>>>* local_queue;
	};
}
