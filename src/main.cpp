#include "os.hpp"
#include "dispatcher.hpp"
#include "render-system.hpp"
#include "vertexbuffer.hpp"
#include "shader.hpp"
#include "multiton.hpp"
#include "voxelvolume.hpp"
#include "transform.hpp"
#include "material.hpp"
#include <glm/gtc/matrix_transform.hpp>

struct CameraMover : public vv::Subscriber < vv::KeyboardEvent > {
	CameraMover() {
		vv::Dispatcher<vv::KeyboardEvent>::GetInstance()->Subscribe(this);
	}
	void Notify(const vv::KeyboardEvent* data) {
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
						vv::RenderSystem::QueueCommand(vv::VIEW_ACTIVATE, 1);
						break;
				}
				vv::RenderSystem::QueueCommand(vv::MODEL_MATRIX_UPDATE, 1);
				break;
			default:
				break;
		}
	}
};

int main(int argc, void* argv) {
	vv::OS os;

	os.InitializeWindow(800, 600, "VoxelVolution 0.1", 3, 2);

	vv::RenderSystem rs;

	rs.SetViewportSize(800, 600);

	vv::VoxelVolume voxvol;

	auto s = std::make_shared<vv::Shader>();
	s->LoadFromFile(vv::Shader::VERTEX, "basic.vert");
	s->LoadFromFile(vv::Shader::FRAGMENT, "basic.frag");
	s->Build();
	vv::ShaderMap::Set("shader1", s);

	auto basic_fill = std::make_shared<vv::Material>(s);
	vv::MaterialMap::Set("material_basic", basic_fill);

	auto s_overlay = std::make_shared<vv::Shader>();
	s_overlay->LoadFromFile(vv::Shader::VERTEX, "basic.vert");
	s_overlay->LoadFromFile(vv::Shader::FRAGMENT, "overlay.frag");
	s_overlay->Build();
	vv::ShaderMap::Set("shader_overlay", s_overlay);

	auto overlay = std::make_shared<vv::Material>(s_overlay);
	overlay->SetFillMode(GL_LINE);
	vv::MaterialMap::Set("material_overlay", overlay);

	auto voxvol_transform = std::make_shared<vv::Transform>();
	vv::TransformMap::Set(100, voxvol_transform);
	auto vb = std::make_shared<vv::VertexBuffer>();
	vv::VertexBufferMap::Set(100, vb);

	vv::VoxelVolume::QueueCommand<vv::VoxelCommand, std::tuple<std::int16_t, std::int16_t, std::int16_t>>
		(vv::VOXEL_ADD, 100, nullptr, std::make_tuple(0, 1, 1));
	vv::VoxelVolume::QueueCommand<vv::VoxelCommand, std::tuple<std::int16_t, std::int16_t, std::int16_t>>
		(vv::VOXEL_ADD, 100, nullptr, std::make_tuple(0, -1, 1));
	vv::VoxelVolume::QueueCommand<vv::VoxelCommand, std::tuple<std::int16_t, std::int16_t, std::int16_t>>
		(vv::VOXEL_ADD, 100, nullptr, std::make_tuple(0, -1, 0));
	vv::VoxelVolume::QueueCommand<vv::VoxelCommand, std::tuple<std::int16_t, std::int16_t, std::int16_t>>
		(vv::VOXEL_ADD, 100, nullptr, std::make_tuple(0, -1, -1));
	vv::VoxelVolume::QueueCommand<vv::VoxelCommand, std::tuple<std::int16_t, std::int16_t, std::int16_t>>
		(vv::VOXEL_ADD, 100, nullptr, std::make_tuple(1, -1, 1));

	voxvol.Update(0.0);
	vb->Buffer(voxvol.GetVertexBuffer(), voxvol.GetIndexBuffer());
	rs.AddVertexBuffer(basic_fill, vb, 100);
	rs.AddVertexBuffer(overlay, vb, 100);

	auto vb2 = std::make_shared<vv::VertexBuffer>();
	vv::VertexBufferMap::Set(1, vb2);
	vb2->Buffer(voxvol.GetVertexBuffer(), voxvol.GetIndexBuffer());
	rs.AddVertexBuffer(basic_fill, vb2, 1);

	auto camera_transform = std::make_shared<vv::Transform>();
	vv::TransformMap::Set(1, camera_transform);
	vv::RenderSystem::QueueCommand(vv::VIEW_ACTIVATE, 1);
	vv::RenderSystem::QueueCommand(vv::MODEL_MATRIX_ADD, 1);

	auto camera_transform2 = std::make_shared<vv::Transform>();
	vv::TransformMap::Set(2, camera_transform2);
	vv::RenderSystem::QueueCommand(vv::MODEL_MATRIX_ADD, 2);

	CameraMover cam_mover;

	while (!os.Closing()) {
		rs.Update(os.GetDeltaTime());
		os.OSMessageLoop();
		os.SwapBuffers();
	}

	return 0;
}
