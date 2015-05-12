#pragma once

#include <map>
#include <list>
#include <memory>
#include <cstdint>
#include "types.hpp"

namespace vv {
	template <typename T>
	struct ComponentUpdateList {
		frame_id_t frame = 0; // The frame id the changes are in.
		std::map<eid, std::shared_ptr<T>> updates; // Components to be updated
		std::list<eid> removals; // Components to be removed
	};
	};

	// Schedules updates and removals of component's and applies them to
	// the central component store when UpdateTo is called.
	template <typename T>
	class ComponentUpdateSystem {
	public:
		ComponentUpdateSystem() { }
		~ComponentUpdateSystem() { }

		static const std::map<GUID, frame_id_t>& GetUpdatedOnList() {
			return frame_id_updated_on;
		}

		// Applies all future_update_lists up to and including frame_id
		static void UpdateTo(frame_id_t frame_id) {
			for (frame_id_t i = base_frame_id; i <= frame_id; ++i) {
				if (future_update_lists.find(i) != future_update_lists.end()) {
					auto current_frame_list = future_update_lists.at(i);

					auto updates = current_frame_list->updates;
					for (auto pair : updates) {
						Multiton<eid, std::shared_ptr<T>>::Set(pair.first, pair.second);
					}

					auto removals = current_frame_list->removals;
					for (auto entity_id : removals) {
						Multiton<eid, std::shared_ptr<T>>::Remove(entity_id);
					}
					future_update_lists.erase(i);
				}
			}
			base_frame_id = frame_id;
		}

		// Schedule a component update for frame_id or the most future if frame_id is 0.
		static void SubmitUpdate(eid entity_id, std::shared_ptr<T> value, frame_id_t frame_id = 0) {
			if (frame_id > base_frame_id) {
				if (future_update_lists.find(frame_id) == future_update_lists.end()) {
					future_update_lists[frame_id] = std::make_shared<ComponentUpdateList<T>>();
					future_update_lists[frame_id]->frame = frame_id;
				}
				future_update_lists[frame_id]->updates[entity_id] = value;
			}
			else if (frame_id == 0) {
				if (future_update_lists.upper_bound(base_frame_id) == future_update_lists.end()) {
					frame_id = base_frame_id + 1;
					future_update_lists[frame_id] = std::make_shared<ComponentUpdateList<T>>();
					future_update_lists[frame_id]->frame = frame_id;
				}
				else {
					frame_id = (*future_update_lists.upper_bound(base_frame_id)).first;
				}
				(*future_update_lists.upper_bound(base_frame_id)).second->updates[entity_id] = value;
			}
		}

		// Schedule a component removal for frame_id or the most future if frame_id is 0.
		static void SubmitRemoval(eid entity_id, frame_id_t frame_id = 0) {
			if (frame_id > base_frame_id) {
				if (future_update_lists.find(frame_id) == future_update_lists.end()) {
					future_update_lists[frame_id] = std::make_shared<ComponentUpdateList<T>>();
					future_update_lists[frame_id]->frame = frame_id;
				}
				future_update_lists[frame_id]->removals.push_back(entity_id);
			}
			else if (frame_id == 0) {
				if (future_update_lists.upper_bound(base_frame_id) == future_update_lists.end()) {
					frame_id = base_frame_id + 1;
					future_update_lists[frame_id] = std::make_shared<ComponentUpdateList<T>>();
					future_update_lists[frame_id]->frame = frame_id;
				}
				else {
					frame_id = (*future_update_lists.upper_bound(base_frame_id)).first;
				}
				future_update_lists[frame_id]->removals.push_back(entity_id);
			}
		}
	protected:

		// Maps entity ID to the frame_id this component was updated in.
		// TODO Make this a shared_ptr to allow creating a new list and leaving the old
		// for current readers to use unmodified.
		static std::map<GUID, frame_id_t> frame_id_updated_on;
		static std::map<frame_id_t, std::shared_ptr<ComponentUpdateList<T>>> future_update_lists;

		// The last confirmed frame_id
		static frame_id_t base_frame_id;
	};

	template <typename T>
	std::map<frame_id_t, std::shared_ptr<ComponentUpdateList<T>>> ComponentUpdateSystem<T>::future_update_lists;

	template <typename T>

	template <typename T>
	std::map<GUID, frame_id_t> ComponentUpdateSystem<T>::frame_id_updated_on;
	frame_id_t ComponentUpdateSystem<T>::base_frame_id = 0;
}
