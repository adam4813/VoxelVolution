#pragma once


#include <map>
#include <list>
#include <queue>
#include <set>
#include <memory>
#include <cstdint>

namespace vv {
	typedef std::int64_t frame_id_t;

	template <typename T>
	struct StateList {
		frame_id_t frame_id;
		std::list<std::pair<GUID, std::shared_ptr<T>>> list;
		std::map<GUID, std::shared_ptr<T>> components;
	};

	template <typename T>
	class UpdatePublisher;

	template <typename T>
	class StateListQueue {
	public:
		StateListQueue() : current_frame_id(0) {
			UpdatePublisher<T>::Subscribe(this);
		}

		// Get the next list in the queue. If there isn't one return nullptr.
		std::shared_ptr<StateList<T>> GetNextList() {
			if (this->state_lists.size() > 0) {
				std::shared_ptr<StateList<T>> next_list(this->state_lists.front());
				this->current_list = next_list;
				this->state_lists.pop();
				this->current_frame_id = next_list->frame_id;
				return next_list;
			}

			return nullptr;
		}

		std::shared_ptr<StateList<T>> GetCurrentList() {
			return this->current_list;
		}

		template <typename T>
		friend class UpdatePublisher;
	protected:
		// Stores a queue of update lists.
		std::queue<std::shared_ptr<StateList<T>>> state_lists;
		std::shared_ptr<StateList<T>> current_list;
		frame_id_t current_frame_id;
	};


	template <typename T>
	class UpdatePublisher {
	public:
		UpdatePublisher() { }

		static void Initialize() {
			for (frame_id_t frame_id = 0; frame_id <= 10; ++frame_id) {
				state_lists[frame_id] = std::make_shared<StateList<T>>();
				state_lists[frame_id]->frame_id = frame_id;
			}
			current_frame_id = 0;
			current_state_list = state_lists.at(0);
		}

		static void EndFrame(frame_id_t frame_id) {
			if (state_lists.find(frame_id) != state_lists.end()) {
				auto state_list = state_lists.at(frame_id);
				for (auto consumer : consumers) {
					consumer->state_lists.push(state_list);
				}
			}
		}

		static void StartFrame() {
			for (std::pair<frame_id_t, std::weak_ptr<StateList<T>>> sl : state_lists) {
				if (sl.second.use_count() == 1) {
					current_state_list = sl.second.lock();
					++current_frame_id;
					current_state_list->frame_id = current_frame_id;
					return;
				}
			}
		}

		static void Subscribe(StateListQueue<T>* c) {
			consumers.push_back(c);
		}

		static void SubmitUpdate(GUID entity_id, std::shared_ptr<T> value, frame_id_t frame_id = 0) {
			current_state_list->components[entity_id] = value;
		}

		static void SubmitRemoveal(GUID entity_id, frame_id_t frame_id = 0) {
			current_state_list->components.erase(entity_id);
		}
	private:
		static std::map<frame_id_t, std::shared_ptr<StateList<T>>> state_lists;
		static std::list<StateListQueue<T>*> consumers;
		static std::shared_ptr<StateList<T>> current_state_list;
		static frame_id_t current_frame_id;
	};
	template <typename T>
	std::map<frame_id_t, std::shared_ptr<StateList<T>>> UpdatePublisher<T>::state_lists;

	template <typename T>
	std::list<StateListQueue<T>*> UpdatePublisher<T>::consumers;

	template <typename T>
	std::shared_ptr<StateList<T>> UpdatePublisher<T>::current_state_list;

	template <typename T>
	frame_id_t UpdatePublisher<T>::current_frame_id = 0;
}
