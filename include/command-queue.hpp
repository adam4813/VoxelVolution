#pragma once

#include <queue>
#include <memory>
#include <functional>
#include <atomic>
#include "multiton.hpp"

namespace vv {	
	template <class T>
	struct Command {
		Command(std::function<void(T*)>&& command) : command(std::move(command)) { }
		Command(Command&& c) : command(std::move(c.command)) { }
		std::function<void(T*)> command;
	};

	template <class T>
	class CommandQueue {
	public:
		CommandQueue() : local_queue(new std::queue<Command<T>>()) { }
		~CommandQueue() { }

		void ProcessCommandQueue() {
			this->local_queue = global_queue.exchange(this->local_queue);

			while (!this->local_queue->empty()) {
				auto command = this->local_queue->front();
				this->local_queue->pop();

				command.command(static_cast<T*>(this));
			}
		}

		static void QueueCommand(Command<T> command) {
			(*global_queue).push(std::move(command));
		}

		static void QueueCommand(std::function<void(T*)>&& command) {
			(*global_queue).push(std::move(command));
		}
	protected:
		// Each derived class must define global_queue in its source file.
		static std::atomic<std::queue<Command<T>>*> global_queue;
		std::queue<Command<T>>* local_queue;
	};
}
