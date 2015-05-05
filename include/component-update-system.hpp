#pragma once

#include <map>
#include <list>
#include <memory>
#include <cstdint>

namespace vv {
	typedef std::int64_t frame_id_t;

	template <typename T>
	struct ComponentUpdateList {
		frame_id_t frame = 0; // The frame id the changes are in.
		std::map<GUID, std::shared_ptr<T>> updates; // Components to be updated
		std::list<GUID> removals; // Components to be removed
	};

	template <typename T>
	class ComponentUpdateSystem {
	public:
		ComponentUpdateSystem() { }
		~ComponentUpdateSystem() { }

		static const std::map<GUID, frame_id_t>& GetUpdatedOnList() {
			return frame_id_updated_on;
		}

		// Applies all future_updates up to and including frame_id
		static void UpdateTo(frame_id_t frame_id) {
			for (frame_id_t i = confirmed_frame_id; i <= frame_id; ++i) {
				if (future_updates.find(i) != future_updates.end()) {
					auto current_frame_list = future_updates.at(i);

					auto updates = current_frame_list->updates;
					for (auto pair : updates) {
						Entity(pair.first).Set<T>(pair.second);
						frame_id_updated_on[pair.first] = frame_id;
					}

					auto removals = current_frame_list->removals;
					for (auto entity_id : removals) {
						Entity(entity_id).Remove<T>();
					}
					future_updates.erase(i);
				}
			}
			confirmed_frame_id = frame_id;
		}

		static void SubmitUpdate(GUID entity_id, std::shared_ptr<T> value, frame_id_t frame_id = 0) {
			if (frame_id > confirmed_frame_id) {
				if (future_updates.find(frame_id) == future_updates.end()) {
					future_updates[frame_id] = std::make_shared<ComponentUpdateList<T>>();
					future_updates[frame_id]->frame = frame_id;
				}
				future_updates[frame_id]->updates[entity_id] = value;
			}
			else if (frame_id == 0) {
				if (future_updates.upper_bound(confirmed_frame_id) == future_updates.end()) {
					frame_id = confirmed_frame_id + 1;
					future_updates[frame_id] = std::make_shared<ComponentUpdateList<T>>();
					future_updates[frame_id]->frame = frame_id;
				}
				else {
					frame_id = (*future_updates.upper_bound(confirmed_frame_id)).first;
				}
				(*future_updates.upper_bound(confirmed_frame_id)).second->updates[entity_id] = value;
			}
		}

		static void SubmitRemoveal(GUID entity_id, frame_id_t frame_id = 0) {
			if (frame_id > confirmed_frame_id) {
				if (future_updates.find(frame_id) == future_updates.end()) {
					future_updates[frame_id] = std::make_shared<ComponentUpdateList<T>>();
					future_updates[frame_id]->frame = frame_id;
				}
				future_updates[frame_id]->removals.push_back(entity_id);
			}
			else if (frame_id == 0) {
				if (future_updates.upper_bound(confirmed_frame_id) == future_updates.end()) {
					frame_id = confirmed_frame_id + 1;
					future_updates[frame_id] = std::make_shared<ComponentUpdateList<T>>();
					future_updates[frame_id]->frame = frame_id;
				}
				else {
					frame_id = (*future_updates.upper_bound(confirmed_frame_id)).first;
				}
				future_updates[frame_id]->removals.push_back(entity_id);
			}
		}
	protected:
		static std::map<frame_id_t, std::shared_ptr<ComponentUpdateList<T>>> future_updates;

		// Maps entity ID to the frame_id this component was updated in.
		// TODO Make this a shared_ptr to allow creating a new list and leaving the old
		// for current readers to use unmodified.
		static std::map<GUID, frame_id_t> frame_id_updated_on;

		// The last confirmed frame_id
		static frame_id_t confirmed_frame_id;
	};

	template <typename T>
	std::map<frame_id_t, std::shared_ptr<ComponentUpdateList<T>>> ComponentUpdateSystem<T>::future_updates;

	template <typename T>
	frame_id_t ComponentUpdateSystem<T>::confirmed_frame_id = 0;

	template <typename T>
	std::map<GUID, frame_id_t> ComponentUpdateSystem<T>::frame_id_updated_on;
}
