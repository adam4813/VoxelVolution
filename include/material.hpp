#pragma once

#include <memory>

#ifndef __APPLE__
#include <GL/glew.h>
#else
#include <OpenGL/gl3.h>
#endif

namespace vv {
	class Shader;

	typedef Multiton<std::string, std::shared_ptr<Material>> MaterialMap;

	class Material {
	public:
		Material(const std::weak_ptr<Shader> shader) : shader(shader), fill_mode(GL_FILL) { }

		GLenum GetFillMode() {
			return this->fill_mode;
		}

		void SetFillMode(const GLenum mode) {
			switch (mode) {
			case GL_LINE:
			case GL_FILL:
			this->fill_mode = mode;
			break;
			default:
			this->fill_mode = GL_FILL;
			}
		}

		std::weak_ptr<Shader> GetShader() {
			return this->shader;
		}
	private:
		GLenum fill_mode;
		std::weak_ptr<Shader> shader;
	};
}
