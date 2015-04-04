#pragma once

#include <queue>
#include <memory>
#include <functional>
#include <atomic>
#include "multiton.hpp"

namespace vv {
	// Container for callback type erasure.
	struct CallbackHolder {
	protected:
		CallbackHolder() { }
	};

	// Used to create callbacks for command completion.
	// Usage: 
	//	auto callback = std::make_shared<Callback<arg_types>([] (arg_types arg1) { });
	//	QueueCommand(COMMAND, entity_id, callback);
	template <typename...args>
	struct Callback : CallbackHolder {
		Callback(std::function<void(args...)> callback) : callback(callback) { }
		std::function<void(args...)> callback;
	};

	// Base class used for commands. T is the command id's type (e.g. enum, int, char).
	template <typename T>
	struct Command {
		Command(T c, GUID entity_id, std::shared_ptr<CallbackHolder> callback = nullptr) :
			command(c), entity_id(entity_id), callback(callback) { }
		T command;
		GUID entity_id;
		std::shared_ptr<CallbackHolder> callback;
	};

	// T is the command id's type (e.g. enum, int, char).
	template <class T>
	class CommandQueue {
	public:
		CommandQueue() : local_queue(new std::queue<std::shared_ptr<Command<T>>>()) { }
		~CommandQueue() { }

		// U is the derived Command type and V is the type of the data passed to that command's constructor.
		// TODO: Possibly change V to be variadic.
		template <typename U, typename V>
		static void QueueCommand(const T c, const GUID entity_id,
			std::shared_ptr<CallbackHolder> callback = nullptr, V data = nullptr) {
			auto command = std::make_shared<U>(c, entity_id, callback, data);
			(*global_queue).push(command);
		}

		static void QueueCommand(const T c, const GUID entity_id,
			std::shared_ptr<CallbackHolder> callback = nullptr) {
			auto command = std::make_shared<Command<T>>(c, entity_id, callback);
			(*global_queue).push(command);
		}
	protected:
		// Each derived class must define global_queue in its source file.
		static std::atomic<std::queue<std::shared_ptr<Command<T>>>*> global_queue;
		std::queue<std::shared_ptr<Command<T>>>* local_queue;
	};
}
