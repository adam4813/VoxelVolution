#pragma once

#include <queue>
#include <memory>
#include <functional>
#include <atomic>
#include "multiton.hpp"

namespace vv {	
	template <class T>
	struct Command {
		Command(std::function<void(T*)> command) : command(command) { }
		std::function<void(T*)> command;
	};

	template <class T>
	class CommandQueue {
	public:
		CommandQueue() : local_queue(new std::queue<std::shared_ptr<Command<T>>>()) { }
		~CommandQueue() { }

		void ProcessCommandQueue() {
			this->local_queue = global_queue.exchange(this->local_queue);

			while (!this->local_queue->empty()) {
				auto command = this->local_queue->front();
				this->local_queue->pop();

				command->command(static_cast<T*>(this));
			}
		}

		static void QueueCommand(std::shared_ptr<Command<T>> command) {
			(*global_queue).push(command);
		}
	protected:
		// Each derived class must define global_queue in its source file.
		static std::atomic<std::queue<std::shared_ptr<Command<T>>>*> global_queue;
		std::queue<std::shared_ptr<Command<T>>>* local_queue;
	};
}
