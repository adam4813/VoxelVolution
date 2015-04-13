#pragma once

#include <list>
#include <map>
#include <memory>
#include <mutex>

#include "multiton.hpp"

namespace vv {
	// Event receiver base class
	template <typename T>
	class Receiver {
	public:
		virtual void On(const GUID entity_id, const T* data) { }
		virtual void On(const T* data) { }

		virtual void On(const GUID entity_id, const T data) { }
		virtual void On(const T data) { }
	};

	// Sends events to receiver's On(...) methods by calling Emit().
	template <typename T>
	class EventSystem final {
	private:
		EventSystem() { }
		EventSystem(const EventSystem& right) {
			instance = right.instance;
		}
		EventSystem& operator=(const EventSystem& right) {
			if (this != &right) {
				instance = right.instance;
			}

			return *this;
		}
		static std::once_flag only_one;
		static std::shared_ptr<EventSystem<T>> instance;
	public:
		static std::shared_ptr<EventSystem<T>> Get() {
			std::call_once(EventSystem::only_one,
				[ ] () {
				EventSystem<T>::instance.reset(new EventSystem<T>());
			}
			);

			return EventSystem<T>::instance;
		}
		~EventSystem() { }

		/**
		 * \brief Subscribes to be notified of data change events.
		 *
		 * \param const unsigned int entity_id ID of the entity to subscribe to.
		 * \param const Receiver<T>* subscriber The subscriber to add.
		 * \return void
		 */
		void Subscribe(const GUID entity_id, Receiver<T>* subscriber) {
			auto sube = this->receivers.find(entity_id);
			if (sube == this->receivers.end()) {
				// no subscriptions for entity, add the entity and subscriber
				this->receivers[entity_id].push_back(subscriber);
				return;
			}
			else {
				// check if subscriber already exists
				for (auto sub : sube->second) {
					if (sub == subscriber) {
						return; // already subscribed
					}
				}
				sube->second.push_back(subscriber);
			}
		}

		/**
		 * \brief Subscribes to be notified of data change events for any entity ID.
		 *
		 * \param const Receiver<T>* subscriber The subscriber to add.
		 * \return void
		 */
		void Subscribe(Receiver<T>* subscriber) {
			this->receivers[0].push_back(subscriber);
		}

		/**
		 * \brief Unsubscribes to notification of data change events.
		 *
		 * \param const unsigned int entity_id ID of the entity to unsubscribe from.
		 * \param const Receiver<T>* subscriber The subscriber to remove.
		 * \return void
		 */
		void Unsubscribe(const GUID entity_id, Receiver<T>* subscriber) {
			if (this->receivers.find(entity_id) != this->receivers.end()) {
				this->receivers[entity_id].remove(subscriber);
			}
		}

		/**
		 * \brief Unsubscribes to notification of data change events.
		 *
		 * \param const Receiver<T>* subscriber The subscriber to remove.
		 * \return void
		 */
		void Unsubscribe(Receiver<T>* subscriber) {
			if (this->receivers.find(0) != this->receivers.end()) {
				this->receivers[0].remove(subscriber);
			}
		}

		/**
		 * \brief Called to notify all subscribers that the data has changed.
		 *
		 * \param const unsigned int entity_id ID of the entity to update.
		 * \param const T* data The changed data.
		 * \return void
		 */
		void Emit(const GUID entity_id, const T* data) {
			if (this->receivers.find(entity_id) != this->receivers.end()) {
				auto subscriber_list = this->receivers.at(entity_id);
				for (Receiver<T>* subscriber : subscriber_list) {
					subscriber->On(entity_id, data);
				}
			}

			if (this->receivers.find(0) != this->receivers.end()) {
				auto subscriber_list = this->receivers.at(0);
				for (Receiver<T>* subscriber : subscriber_list) {
					subscriber->On(entity_id, data);
				}
			}
		}
		void Emit(const GUID entity_id, const T data) {
			if (this->receivers.find(entity_id) != this->receivers.end()) {
				auto subscriber_list = this->receivers.at(entity_id);
				for (Receiver<T>* subscriber : subscriber_list) {
					subscriber->On(entity_id, data);
				}
			}

			if (this->receivers.find(0) != this->receivers.end()) {
				auto subscriber_list = this->receivers.at(0);
				for (Receiver<T>* subscriber : subscriber_list) {
					subscriber->On(entity_id, data);
				}
			}
		}

		/**
		 * \brief Called to notify all subscribers for entity ID 0 that the data has changed.
		 *
		 * \param const T* data The changed data.
		 * \return void
		 */
		void Emit(const T* data) {
			if (this->receivers.find(0) != this->receivers.end()) {
				auto subscriber_list = this->receivers.at(0);
				for (Receiver<T>* subscriber : subscriber_list) {
					subscriber->On(data);
				}
			}
		}
		void Emit(const T data) {
			if (this->receivers.find(0) != this->receivers.end()) {
				auto subscriber_list = this->receivers.at(0);
				for (Receiver<T>* subscriber : subscriber_list) {
					subscriber->On(data);
				}
			}
		}
	private:
		std::map<GUID, std::list<Receiver<T>*>> receivers;
	};

	template<typename T>
	std::once_flag EventSystem<T>::only_one;

	template<typename T>
	std::shared_ptr<EventSystem<T>> EventSystem<T>::instance = nullptr;
}
