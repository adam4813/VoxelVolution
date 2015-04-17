#include "render-system.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
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

	RenderSystem::RenderSystem() : current_view(0) {
		auto err = glGetError();
		// If there is an error that means something went wrong when creating the context.
		if (err) {
			return;
		}
		// Use the GL3 way to get the version number.
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

	void RenderSystem::Update(const double delta) {
		EventQueue<TransformChangedEvent>::ProcessEventQueue();
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

	std::weak_ptr<ModelMatrix> RenderSystem::AddModelMatrix(const GUID entity_id) {
		auto model_matrix = ModelMatrixMap::Get(entity_id);
		auto transform = TransformMap::Get(entity_id);

		// If the model matrix doesn't exist, but a transform does make the model matrix.
		if (!model_matrix && transform) {
			model_matrix = std::make_shared<ModelMatrix>();
			ModelMatrixMap::Set(entity_id, model_matrix);
		}

		auto camera_translation = transform->GetTranslation();
		auto camera_orientation = transform->GetOrientation();
		model_matrix->transform = glm::translate(glm::mat4(1.0), camera_translation) *
			glm::mat4_cast(camera_orientation);

		// Check if there is a view associated with the entity_id and update it as well.
		if (this->views.find(entity_id) != this->views.end()) {
			UpdateViewMatrix(entity_id);
		}

		return model_matrix;
	}

	void RenderSystem::RemoveModelMatrix(const GUID entity_id) {
		ModelMatrixMap::Remove(entity_id);

		// If there was an associated view matrix attempt to remove it as well.
		RemoveViewMatrix(entity_id);
	}

	void RenderSystem::RemoveViewMatrix(const GUID entity_id) {
		if (this->views.find(entity_id) != this->views.end()) {
			this->views.erase(entity_id);
			if (this->views.size() > 0) {
				this->current_view = this->views.begin()->first;
			}
			else {
				this->current_view = 0;
			}
		}
	}
	void RenderSystem::UpdateViewMatrix(const GUID entity_id) {
		auto model_matrix = ModelMatrixMap::Get(entity_id);

		if (!model_matrix) {
			return;
		}
		this->views[entity_id] = glm::inverse(model_matrix->transform);
	}

	bool RenderSystem::ActivateView(const GUID entity_id) {
		if (this->views.find(entity_id) != this->views.end()) {
			this->current_view = entity_id;
			return true;
		}
		return false;
	}

	void RenderSystem::On(const GUID entity_id, std::shared_ptr<TransformChangedEvent> tce_event) {
		auto model_matrix = ModelMatrixMap::Get(tce_event->entity_id);
		if (model_matrix) {
			if (tce_event->frame >= model_matrix->frame) {
				auto camera_translation = tce_event->current.GetTranslation();
				auto camera_orientation = tce_event->current.GetOrientation();
				model_matrix->transform = glm::translate(glm::mat4(1.0), camera_translation) *
					glm::mat4_cast(camera_orientation);

				// Check if there is a view associated with the entity_id and update it as well.
				if (this->views.find(tce_event->entity_id) != this->views.end()) {
					UpdateViewMatrix(tce_event->entity_id);
				}
			}
		}
	}
}
