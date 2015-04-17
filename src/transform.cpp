#include "transform.hpp"
#include "event-system.hpp"

namespace vv {
	Transform::Transform(GUID entity_id) :
		orientation(glm::quat(1, 0, 0, 0)), scale(1.0f), entity_id(entity_id) { }

	void Transform::Translate(const glm::vec3 amount) {
		auto tce_event = std::make_shared<TransformChangedEvent>();
		tce_event->entity_id = this->entity_id;
		tce_event->old = *this;

		this->translation += amount;

		tce_event->current = *this;
		EventSystem<TransformChangedEvent>::Get()->
			Emit(this->entity_id, tce_event);
	}

	void Transform::Rotate(const glm::vec3 amount) {
		auto tce_event = std::make_shared<TransformChangedEvent>();
		tce_event->entity_id = this->entity_id;
		tce_event->old = *this;

		this->rotation += amount;

		glm::quat change(this->rotation);
		this->orientation = glm::normalize(change * this->orientation);

		tce_event->current = *this;
		EventSystem<TransformChangedEvent>::Get()->
			Emit(this->entity_id, tce_event);
	}

	void Transform::OrientedTranslate(const glm::vec3 amount) {
		auto tce_event = std::make_shared<TransformChangedEvent>();
		tce_event->entity_id = this->entity_id;
		tce_event->old = *this;

		this->translation += this->orientation * amount;

		tce_event->current = *this;
		EventSystem<TransformChangedEvent>::Get()->
			Emit(this->entity_id, tce_event);
	}

	void Transform::OrientedRotate(const glm::vec3 amount) {
		auto tce_event = std::make_shared<TransformChangedEvent>();
		tce_event->entity_id = this->entity_id;
		tce_event->old = *this;

		this->rotation += amount;

		glm::quat qX = glm::angleAxis(amount.x, this->orientation * RIGHT_VECTOR);
		glm::quat qY = glm::angleAxis(amount.y, this->orientation * UP_VECTOR);
		glm::quat qZ = glm::angleAxis(amount.z, this->orientation * glm::vec3(0.0, 0.0, 1.0)); // Z axis not Forward
		glm::quat change = qX * qY * qZ;

		this->orientation = glm::normalize(change * this->orientation);

		tce_event->current = *this;
		EventSystem<TransformChangedEvent>::Get()->
			Emit(this->entity_id, tce_event);
	}

	void Transform::Scale(const glm::vec3 amount) {
		auto tce_event = std::make_shared<TransformChangedEvent>();
		tce_event->entity_id = this->entity_id;
		tce_event->old = *this;

		this->scale *= amount;

		tce_event->current = *this;
		EventSystem<TransformChangedEvent>::Get()->
			Emit(this->entity_id, tce_event);
	}

	void Transform::SetTranslation(const glm::vec3 new_translation) {
		auto tce_event = std::make_shared<TransformChangedEvent>();
		tce_event->entity_id = this->entity_id;
		tce_event->old = *this;

		this->translation = new_translation;

		tce_event->current = *this;
		EventSystem<TransformChangedEvent>::Get()->
			Emit(this->entity_id, tce_event);
	}

	void Transform::SetRotation(const glm::vec3 new_rotation) {
		auto tce_event = std::make_shared<TransformChangedEvent>();
		tce_event->entity_id = this->entity_id;
		tce_event->old = *this;

		this->orientation = glm::normalize(glm::quat(new_rotation));
		this->rotation.x = atan2(2.0f * (this->orientation.y * this->orientation.z +
			this->orientation.w * this->orientation.x),
			this->orientation.w * this->orientation.w - this->orientation.x *
			this->orientation.x - this->orientation.y * this->orientation.y +
			this->orientation.z * this->orientation.z);
		this->rotation.y = glm::asin(-2.0f * (this->orientation.x * this->orientation.z - this->orientation.w * this->orientation.y));
		this->rotation.z = atan2(2.0f * (this->orientation.x * this->orientation.y +
			this->orientation.w * this->orientation.z),
			this->orientation.w * this->orientation.w + this->orientation.x *
			this->orientation.x - this->orientation.y * this->orientation.y -
			this->orientation.z * this->orientation.z);

		tce_event->current = *this;
		EventSystem<TransformChangedEvent>::Get()->
			Emit(this->entity_id, tce_event);
	}

	void Transform::SetOrientation(const glm::quat new_orientation) {
		auto tce_event = std::make_shared<TransformChangedEvent>();
		tce_event->entity_id = this->entity_id;
		tce_event->old = *this;

		this->orientation = new_orientation;
		this->rotation.x = atan2(2.0f * (this->orientation.y * this->orientation.z +
			this->orientation.w * this->orientation.x),
			this->orientation.w * this->orientation.w - this->orientation.x *
			this->orientation.x - this->orientation.y * this->orientation.y +
			this->orientation.z * this->orientation.z);
		this->rotation.y = glm::asin(-2.0f * (this->orientation.x * this->orientation.z - this->orientation.w * this->orientation.y));
		this->rotation.z = atan2(2.0f * (this->orientation.x * this->orientation.y +
			this->orientation.w * this->orientation.z),
			this->orientation.w * this->orientation.w + this->orientation.x *
			this->orientation.x - this->orientation.y * this->orientation.y -
			this->orientation.z * this->orientation.z);

		tce_event->current = *this;
		EventSystem<TransformChangedEvent>::Get()->
			Emit(this->entity_id, tce_event);
	}

	void Transform::SetScale(const glm::vec3 new_scale) {
		auto tce_event = std::make_shared<TransformChangedEvent>();
		tce_event->entity_id = this->entity_id;
		tce_event->old = *this;

		this->scale = new_scale;

		tce_event->current = *this;
		EventSystem<TransformChangedEvent>::Get()->
			Emit(this->entity_id, tce_event);
	}

	glm::vec3 Transform::GetTranslation() const {
		return this->translation;
	}

	glm::vec3 Transform::GetRotation() const {
		return this->rotation;
	}

	glm::quat Transform::GetOrientation() const {
		return this->orientation;
	}

	glm::vec3 Transform::GetScale() const {
		return this->scale;
	}
}
