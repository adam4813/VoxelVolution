#pragma once

#include <memory>
#include <map>
#include <tuple>
#include <cstdint>
#include "multiton.hpp"

namespace vv {
	typedef std::int64_t eid;

	class Entity {
	public:
		Entity(eid id) : id(id) { }

		template <typename T, typename... U>
		void Add(U&&... args) {
			if (!Multiton<eid, std::shared_ptr<T>>::Get(this->id)) {
				auto comp = std::make_shared<T>(std::forward<U>(args)...);
				Set(comp);
			}
		}

		template <typename T>
		std::weak_ptr<T> Get() {
			return Multiton<eid, std::shared_ptr<T>>::Get(this->id);
		}

		template <typename... T>
		std::tuple<std::weak_ptr<T>...> GetList() {
			return std::make_tuple(Multiton<eid, std::shared_ptr<T>>::Get(this->id)...);
		}
	protected:
		template <typename T>
		void Set(std::shared_ptr<T> val) {
			Multiton<eid, std::shared_ptr<T>>::Set(this->id, val);
		}

		template <typename T>
		void Set(T val) {
			auto comp = std::make_shared<T>(val);
			Set(comp);
		}

		template <typename T>
		void Remove() {
			Multiton<eid, std::shared_ptr<T>>::Remove(this->id);
		}

		template <typename T>
		friend class ComponentUpdateSystem;
	private:
		eid id;
	};
}
