#pragma once

#include <memory>
#include <glm/mat4x4.hpp>
#include "multiton.hpp"
#include "event-system.hpp"

namespace vv {
	struct ModelMatrix;

	class Camera {
	public:
		Camera(GUID entity_id);
		~Camera();

		bool MakeActive();

		glm::mat4 GetViewMatrix();
	private:
		std::weak_ptr<ModelMatrix> model_matrix;
		GUID entity_id;
	};

	struct KeyboardEvent;

	// TODO: Create Controller system that calls update on all controller instances.
	struct Controller {
		virtual void Update(double delta) { }
	};
	struct CameraMover : public Controller, public EventQueue < KeyboardEvent > {
		CameraMover(std::shared_ptr<Camera> c) : cam(c) { }

		void On(std::shared_ptr<KeyboardEvent> data);

		void Update(double delta);

		std::shared_ptr<Camera> cam;
	};
}
