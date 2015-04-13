#include "os.hpp"
#include "dispatcher.hpp"
#include "render-system.hpp"
#include "vertexbuffer.hpp"
#include "shader.hpp"
#include "multiton.hpp"
#include "voxelvolume.hpp"
#include "transform.hpp"
#include "material.hpp"
#include "components/camera.hpp"
#include "polygonmeshdata.hpp"
#include <glm/gtc/matrix_transform.hpp>

struct CameraMover : public vv::Subscriber < vv::KeyboardEvent > {
	CameraMover(std::shared_ptr<vv::Camera> c) : cam(c) {
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
						cam->MakeActive();
						break;
				}
				cam->UpdateViewMatrix();
				break;
			default:
				break;
		}
	}

	std::shared_ptr<vv::Camera> cam;
};

int main(int argc, void* argv) {
	vv::OS os;

	os.InitializeWindow(800, 600, "VoxelVolution 0.1", 3, 2);

	vv::RenderSystem rs;

	rs.SetViewportSize(800, 600);

	auto voxvol = vv::VoxelVolume::Create(100, "bob", 0);
	auto voxvol_shared = voxvol.lock();
	auto shader_files = std::list < std::pair<vv::Shader::ShaderType, std::string> > {
		std::make_pair(vv::Shader::VERTEX, "basic.vert"), std::make_pair(vv::Shader::FRAGMENT, "basic.frag"),
	};
	auto s = vv::Shader::CreateFromFile("shader1", shader_files);
	auto basic_fill = vv::Material::Create("material_basic", s);

	shader_files = std::list < std::pair<vv::Shader::ShaderType, std::string> > {
		std::make_pair(vv::Shader::VERTEX, "basic.vert"), std::make_pair(vv::Shader::FRAGMENT, "overlay.frag"),
	};
	auto s_overlay = vv::Shader::CreateFromFile("shader_overlay", shader_files);
	auto overlay = vv::Material::Create("material_overlay", s_overlay);
	overlay.lock()->SetFillMode(GL_LINE);

	auto voxvol_transform = std::make_shared<vv::Transform>();
	vv::TransformMap::Set(100, voxvol_transform);

	vv::VoxelCommand add_voxel(
		[] (vv::VoxelVolume* vox_vol) {
		vox_vol->AddVoxel(0, 1, 1);
		vox_vol->AddVoxel(0, -1, 1);
		vox_vol->AddVoxel(0, -1, 0);
		vox_vol->AddVoxel(0, -1, -1);
		vox_vol->AddVoxel(1, -1, 1);
	});
	vv::VoxelVolume::QueueCommand(add_voxel);

	voxvol_shared->Update(0.0);
	auto mesh = voxvol_shared->GetMesh().lock();
	if (mesh) {
		auto vb = std::make_shared<vv::VertexBuffer>();
		vv::VertexBufferMap::Set(100, vb);
		vb->Buffer(*mesh->GetVertexBuffer(), *mesh->GetIndexBuffer());

		rs.AddVertexBuffer(basic_fill, vb, 100);
		rs.AddVertexBuffer(overlay, vb, 100);

		auto vb2 = std::make_shared<vv::VertexBuffer>();
		vv::VertexBufferMap::Set(1, vb2);
		vb2->Buffer(*mesh->GetVertexBuffer(), *mesh->GetIndexBuffer());

	auto camera_transform = std::make_shared<vv::Transform>();
	vv::TransformMap::Set(1, camera_transform);
	vv::RenderSystem::QueueCommand(vv::VIEW_ACTIVATE, 1);
	vv::RenderSystem::QueueCommand(vv::MODEL_MATRIX_ADD, 1);
		rs.AddVertexBuffer(basic_fill, vb2, 1);
	}

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
