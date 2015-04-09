#pragma once

#include <memory>
#include <list>
#include <map>
#include <glm/mat4x4.hpp>

#ifndef __APPLE__
#include <GL/glew.h>
#else
#include <OpenGL/gl3.h>
#endif

#include "multiton.hpp"
#include "command-queue.hpp"

namespace vv {
	struct VertexBuffer;
	class Material;

	class RenderSystem;
	typedef Command<RenderSystem> RenderCommand;

	struct ModelMatrix {
		glm::mat4 transform;
	};

	typedef Multiton<GUID, std::shared_ptr<ModelMatrix>> ModelMatrixMap;

	class RenderSystem : public CommandQueue < RenderSystem > {
	public:
		RenderSystem();

		void SetViewportSize(const unsigned int width, const unsigned int height);

		void Update(const double delta);

		void AddVertexBuffer(const std::weak_ptr<Material> mat, const std::weak_ptr<VertexBuffer> buffer, const GUID entity_id);

		// Updates a model matrix, if one doesn't exist for the given entity_id it will be created.
		// Returns a weak_ptr to the create model matrix.
		std::weak_ptr<ModelMatrix> UpdateModelMatrix(const GUID entity_id);

		// Remove a model matrix.
		void RemoveModelMatrix(const GUID entity_id);

		// Update a view matrix to be the inverse of its corresponding model matrix.
		// If a model matrix doesn't exist, this fails silently.
		void UpdateViewMatrix(const GUID entity_id);

		// Checks if there is a view associated entity_id and sets it as the current view.
		bool ActivateView(const GUID entity_id);

		// Remove a view matrix.
		void RemoveViewMatrix(const GUID entity_id);
	protected:
		//void CreateVertexBuffer(GUID entity_id, const std::vector<Vertex>& verts, const std::vector<GLuint>& indices);
	private:
		glm::mat4 projection;
		std::map<GUID, glm::mat4> views;
		GUID current_view;
		unsigned int window_width, window_height;
		std::map < std::weak_ptr<Material>, std::pair < std::weak_ptr<VertexBuffer>,
			std::list<GUID >> , std::owner_less<std::weak_ptr<Material>>> buffers;
	};
}
