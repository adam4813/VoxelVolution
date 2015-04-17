#include "components/camera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include "os.hpp"
#include "render-system.hpp"
#include "transform.hpp"

namespace vv {
	void CameraMover::Update(double delta) {
		ProcessEventQueue();
	}

	void CameraMover::On(std::shared_ptr<vv::KeyboardEvent> data) {
		auto transform = vv::TransformMap::Get(1);
		switch (data->action) {
			case vv::KeyboardEvent::KEY_UP:
				switch (data->key) {
					case GLFW_KEY_A:
						transform->OrientedRotate(glm::vec3(0.0, glm::radians(10.0f), 0.0));
						break;
					case GLFW_KEY_D:
						transform->OrientedRotate(glm::vec3(0.0, glm::radians(-10.0f), 0.0));
						break;
					case GLFW_KEY_W:
						transform->OrientedTranslate(glm::vec3(0.0, 0.0, -1.0));
						break;
					case GLFW_KEY_S:
						transform->OrientedTranslate(glm::vec3(0.0, 0.0, 1.0));
						break;
					case GLFW_KEY_SPACE:
						cam->MakeActive();
						break;
				}
				break;
			default:
				break;
		}
	}
	Camera::Camera(GUID entity_id) : entity_id(entity_id) {
		// Add the transform
		auto camera_transform = TransformMap::Get(this->entity_id);
		if (!camera_transform) {
			camera_transform = std::make_shared<Transform>(entity_id);
			TransformMap::Set(this->entity_id, camera_transform);
		}

		// Queue a command to add the view matrix and model matrix
		// associated with this entity_id.
		RenderSystem::QueueCommand([this] (RenderSystem* ren_sys) {
			this->model_matrix = ren_sys->AddModelMatrix(this->entity_id);
			ren_sys->UpdateViewMatrix(this->entity_id);
		});
	}

	Camera::~Camera() {
		RenderSystem::QueueCommand([this] (RenderSystem* ren_sys) {
			ren_sys->RemoveModelMatrix(this->entity_id);
		});
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
