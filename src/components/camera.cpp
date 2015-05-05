#include "components/camera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include "os.hpp"
#include "render-system.hpp"
#include "transform.hpp"
#include "entity.hpp"
#include "component-update-system.hpp"

namespace vv {
	void CameraMover::Update(double delta) {
		ProcessEventQueue();
	}

	void CameraMover::On(std::shared_ptr<vv::KeyboardEvent> data) {
		auto camera = this->cam.lock();
		if (!camera) {
			return;
		}
		auto transforms = Entity(1).GetList<Position, Orientation>();
		if (!std::get<0>(transforms).lock()) {
			return;
		}
		std::shared_ptr<Position> position = std::make_shared<Position>(*std::get<0>(transforms).lock().get());
		if (!std::get<1>(transforms).lock()) {
			return;
		}
		std::shared_ptr<Orientation> orientation = std::make_shared<Orientation>(*std::get<1>(transforms).lock().get());

		switch (data->action) {
			case vv::KeyboardEvent::KEY_UP:
				switch (data->key) {
					case GLFW_KEY_A:
						orientation->OrientedRotate(glm::vec3(0.0, glm::radians(10.0f), 0.0));
						break;
					case GLFW_KEY_D:
						orientation->OrientedRotate(glm::vec3(0.0, glm::radians(-10.0f), 0.0));
						break;
					case GLFW_KEY_W:
						position->OrientedTranslate(orientation->value, glm::vec3(0.0, 0.0, -1.0));
						break;
					case GLFW_KEY_S:
						position->OrientedTranslate(orientation->value, glm::vec3(0.0, 0.0, 1.0));
						break;
					case GLFW_KEY_SPACE:
						camera->MakeActive();
						break;
				}
				break;
			default:
				break;
		}
		ComponentUpdateSystem<Position>::SubmitUpdate(1, position);
		ComponentUpdateSystem<Orientation>::SubmitUpdate(1, orientation);
	}
	Camera::Camera(GUID entity_id) : entity_id(entity_id) {
		// Queue a command to add the view matrix and model matrix
		// associated with this entity_id.
		RenderSystem::QueueCommand([this] (RenderSystem* ren_sys) {
			this->model_matrix = ren_sys->AddModelMatrix(this->entity_id);
			ren_sys->UpdateViewMatrix(this->entity_id);
		});
		auto old_position = Entity(this->entity_id).Get<Position>();
		std::shared_ptr<Position> position;
		if (old_position.lock()) {
			position = std::make_shared<Position>(*old_position.lock().get());
		}
		else {
			position = std::make_shared<Position>();
		}
		auto old_orientation = Entity(this->entity_id).Get<Orientation>();
		std::shared_ptr<Orientation> orientation;
		if (old_orientation.lock()) {
			orientation = std::make_shared<Orientation>(*old_orientation.lock().get());
		}
		else {
			orientation = std::make_shared<Orientation>();
		}
		ComponentUpdateSystem<Position>::SubmitUpdate(1, position);
		ComponentUpdateSystem<Orientation>::SubmitUpdate(1, orientation);
	}

	Camera::~Camera() {
		RenderSystem::QueueCommand([this] (RenderSystem* ren_sys) {
			ren_sys->RemoveModelMatrix(this->entity_id);
		});
		ComponentUpdateSystem<Position>::SubmitRemoveal(this->entity_id);
		ComponentUpdateSystem<Orientation>::SubmitRemoveal(this->entity_id);
	}

	bool Camera::MakeActive() {
		auto model = this->model_matrix.lock();
		if (model) {
			RenderSystem::QueueCommand([this] (RenderSystem* ren_sys) {
				ren_sys->ActivateView(this->entity_id);
			});
			return true;
		}

		return false;
	}

	glm::mat4 Camera::GetViewMatrix() {
		auto model = this->model_matrix.lock();
		if (model) {
			return glm::inverse(model->transform);
		}
		return glm::mat4(1.0);
	}
}
