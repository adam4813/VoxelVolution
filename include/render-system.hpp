#pragma once

#include <memory>
#include <list>
#include <map>
#include <atomic>
#include <queue>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

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

	enum RS_COMMAND {
		VIEW_ADD,
		VIEW_UPDATE,
		VIEW_ACTIVATE,
		VIEW_REMOVE,
		MODEL_MATRIX_ADD,
		MODEL_MATRIX_UPDATE,
		MODEL_MATRIX_REMOVE,
		VB_ADD,
		VB_REMOVE,
	};

	template <typename T>
	struct RenderCommand : Command < RS_COMMAND > {
		RenderCommand(const RS_COMMAND rs_c, const GUID entity_id, std::shared_ptr<CallbackHolder> callback = nullptr, T data = nullptr) :
			Command(rs_c, entity_id, callback), data(data) { }
		T data;
	};

	struct ModelMatrix {
		glm::mat4 transform;
	};

	typedef Multiton<GUID, std::shared_ptr<ModelMatrix>> ModelMatrixMap;

	class RenderSystem : public CommandQueue < RS_COMMAND > {
	public:
		RenderSystem();

		void SetViewportSize(const unsigned int width, const unsigned int height);

		void Update(const double delta);

		void AddVertexBuffer(const std::weak_ptr<Material> mat, const std::weak_ptr<VertexBuffer> buffer, const GUID entity_id);

	protected:
		void ProcessCommandQueue();

		void UpdateModelMatrix(const GUID entity_id);

		void RemoveModelMatrix(const GUID entity_id);

		void UpdateViewMatrix(const GUID entity_id);

		//void CreateVertexBuffer(GUID entity_id, const std::vector<Vertex>& verts, const std::vector<GLuint>& indices);
	private:
		glm::mat4 projection;
		std::map<GUID, glm::mat4> views;
		GUID current_view;
		unsigned int window_width, window_height;
		std::map<std::weak_ptr<Material>, std::pair<std::weak_ptr<VertexBuffer>, std::list<GUID>>, std::owner_less<std::weak_ptr<Material>>> buffers;
	};
}
