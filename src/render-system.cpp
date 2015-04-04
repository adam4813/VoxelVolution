#include "render-system.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <thread>

#include "shader.hpp"
#include "vertexbuffer.hpp"
#include "transform.hpp"
#include "material.hpp"

namespace vv {
	class Texture {
	public:
		GLuint name;
	};

	typedef Multiton<std::string, std::shared_ptr<Texture>> TextureMap;

	std::atomic<std::queue<std::shared_ptr<Command<RS_COMMAND>>>*> RenderSystem::global_queue =
		new std::queue<std::shared_ptr<Command<RS_COMMAND>>>();

	RenderSystem::RenderSystem() : current_view(0) {
		auto err = glGetError();
		if (err) {
			return;
		}
		// Use the GL3 way to get the version number
		int gl_version[3];
		glGetIntegerv(GL_MAJOR_VERSION, &gl_version[0]);
		glGetIntegerv(GL_MINOR_VERSION, &gl_version[1]);
		if (err) {
			return;
		}
		int opengl_version = gl_version[0] * 100 + gl_version[1] * 10;

		if (opengl_version < 300) {
			std::cerr << "OpenGL version " << opengl_version << std::endl;
			assert(opengl_version >= 300);
		}

		SetViewportSize(800, 600);
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	void RenderSystem::SetViewportSize(const unsigned int width, const unsigned int height) {
		this->window_height = height;
		this->window_width = width;

		float aspect_ratio = static_cast<float>(this->window_width) / static_cast<float>(this->window_height);
		if (aspect_ratio < 1.0f) {
			aspect_ratio = 4.0f / 3.0f;
		}

		this->projection = glm::perspective(
			glm::radians(45.0f),
			aspect_ratio,
			0.1f,
			10000.0f
			);
	}

	void RenderSystem::ProcessCommandQueue() {
		this->local_queue = global_queue.exchange(this->local_queue);

		while (!this->local_queue->empty()) {
			auto action = this->local_queue->front();
			this->local_queue->pop();

			switch (action->command) {
				case RS_COMMAND::VIEW_ACTIVATE:
					this->current_view = action->entity_id;
				case RS_COMMAND::VIEW_ADD:
				case RS_COMMAND::VIEW_UPDATE:
					UpdateViewMatrix(action->entity_id);
					break;
				case RS_COMMAND::VIEW_REMOVE:
					this->views.erase(action->entity_id);
					break;
				case RS_COMMAND::MODEL_MATRIX_ADD:
				case RS_COMMAND::MODEL_MATRIX_UPDATE:
					UpdateModelMatrix(action->entity_id);
					{
						auto cast_callback =
							std::static_pointer_cast<Callback<std::weak_ptr<ModelMatrix>>>(action->callback);
						if (cast_callback) {
							cast_callback->callback(ModelMatrixMap::Get(action->entity_id));
						}
					}
					break;
				case RS_COMMAND::MODEL_MATRIX_REMOVE:
					RemoveModelMatrix(action->entity_id);
					break;
				case RS_COMMAND::VB_ADD:
					{
						auto cast_command =
							std::static_pointer_cast<RenderCommand<std::weak_ptr<vv::VertexBuffer>>>(action);
						if (cast_command->data.lock()) {
							cast_command->data.reset();
						}
					}
					break;
			}
		}
	}

	void RenderSystem::Update(const double delta) {
		ProcessCommandQueue();
		static float red = 0.3f, blue = 0.3f, green = 0.3f;

		glClearColor(red, green, blue, 1.0f);
		glViewport(0, 0, this->window_width, this->window_height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		auto camera_matrix = this->views[this->current_view];

		for (auto material_group : this->buffers) {
			auto material = material_group.first.lock();
			if (!material) {
				continue;
			}
			glPolygonMode(GL_FRONT_AND_BACK, material->GetFillMode());
			auto shader = material->GetShader().lock();
			if (!material) {
				continue;
			}
			shader->Use();

			glUniformMatrix4fv(shader->GetUniform("view"), 1, GL_FALSE, &camera_matrix[0][0]);
			glUniformMatrix4fv(shader->GetUniform("projection"), 1, GL_FALSE, &this->projection[0][0]);
			GLint model_index = shader->GetUniform("model");

			auto vb = material_group.second.first.lock();
			if (!vb) {
				continue;
			}
			glBindVertexArray(vb->vao);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vb->ibo);
			/*for (size_t i = 0; i < mesh_group.second.textures.size(); ++i) {
				auto tex = mesh_group.second.textures[i].lock();
				GLuint name = 0;
				if (tex) {
				name = tex->name;
				}
				shader->ActivateTextureUnit(i, name);
				}*/

			for (GUID entity_id : material_group.second.second) {
				auto transform = ModelMatrixMap::Get(entity_id);
				if (transform) {
					glUniformMatrix4fv(model_index, 1, GL_FALSE,
						&transform->transform[0][0]);
				}
				else {
					static glm::mat4 identity(1.0);
					glUniformMatrix4fv(model_index, 1, GL_FALSE,
						&identity[0][0]);
				}
				/*auto renanim = ren_group.animations.find(entity_id);
				if (renanim != ren_group.animations.end()) {
				glUniform1i(u_animate_loc, 1);
				auto &animmatricies = renanim->second->matrices;
				glUniformMatrix4fv(u_animatrix_loc, animmatricies.size(), GL_FALSE, &animmatricies[0][0][0]);
				}
				else {
				glUniform1i(u_animate_loc, 0);
				}*/
				glDrawElements(GL_TRIANGLES, vb->index_count, GL_UNSIGNED_INT, 0);
			}

			shader->UnUse();
		}
	}

	void RenderSystem::AddVertexBuffer(const std::weak_ptr<Material> mat,
		const std::weak_ptr<VertexBuffer> buffer, const GUID entity_id) {
		auto mat1 = mat.lock();
		for (auto material_group : this->buffers) {
			auto mat2 = material_group.first.lock();
			if ((mat1 && mat2) && (mat1 == mat2)) {
				auto vb1 = buffer.lock();
				auto vb2 = material_group.second.first.lock();
				if ((vb1 && vb2) && (vb1 == vb2)) {
					material_group.second.second.push_back(entity_id);
					return;
				}
			}
		}
		std::list<GUID> entity_list;
		entity_list.push_back(entity_id);
		this->buffers[mat] = std::make_pair(buffer, std::move(entity_list));
	}

	void RenderSystem::UpdateModelMatrix(const GUID entity_id) {
		auto model_matrix = ModelMatrixMap::Get(entity_id);
		auto transform = TransformMap::Get(entity_id);
		if (!model_matrix && transform) {
			model_matrix = std::make_shared<ModelMatrix>();
			ModelMatrixMap::Set(entity_id, model_matrix);
		}
		else if (!transform) {
			return;
		}

		auto camera_translation = transform->GetTranslation();
		auto camera_orientation = transform->GetOrientation();
		model_matrix->transform = glm::translate(glm::mat4(1.0), camera_translation) *
			glm::mat4_cast(camera_orientation);
		if (this->views.find(entity_id) != this->views.end()) {
			UpdateViewMatrix(entity_id);
		}
	}

	void RenderSystem::RemoveModelMatrix(const GUID entity_id) {
		ModelMatrixMap::Remove(entity_id);
	}

	void RenderSystem::UpdateViewMatrix(const GUID entity_id) {
		auto model_matrix = ModelMatrixMap::Get(entity_id);
		if (!model_matrix) {
			return;
		}
		this->views[entity_id] = glm::inverse(model_matrix->transform);
	}
}
