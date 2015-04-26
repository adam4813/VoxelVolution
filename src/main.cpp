#include "os.hpp"
#include "event-system.hpp"
#include "event-queue.hpp"
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
		[ ] (vv::VoxelVolume* vox_vol) {
		vox_vol->AddVoxel(0, 1, 1);
		vox_vol->AddVoxel(0, -1, 1);
		vox_vol->AddVoxel(0, -1, 0);
		vox_vol->AddVoxel(0, -1, -1);
		vox_vol->AddVoxel(1, -1, 1);
	});
	vv::VoxelVolume::QueueCommand(std::move(add_voxel));
	voxvol_shared->Update(0.0);

	vv::RenderCommand add_vb(
		[voxvol_shared, basic_fill, overlay] (vv::RenderSystem* ren_sys) {
		auto mesh = voxvol_shared->GetMesh().lock();
		if (mesh) {
			auto vb = std::make_shared<vv::VertexBuffer>();
			vv::VertexBufferMap::Set(100, vb);
			vb->Buffer(*mesh->GetVertexBuffer(), *mesh->GetIndexBuffer());

			ren_sys->AddVertexBuffer(basic_fill, vb, 100);
			ren_sys->AddVertexBuffer(overlay, vb, 100);

			auto vb2 = std::make_shared<vv::VertexBuffer>();
			vv::VertexBufferMap::Set(1, vb2);
			vb2->Buffer(*mesh->GetVertexBuffer(), *mesh->GetIndexBuffer());

			ren_sys->AddVertexBuffer(basic_fill, vb2, 1);
		}
	});
	vv::RenderSystem::QueueCommand(std::move(add_vb));


	std::shared_ptr<vv::Camera> cam1 = std::make_shared<vv::Camera>(1);
	vv::Camera cam2(2);

	vv::CameraMover cam_mover(cam1);

	while (!os.Closing()) {
		rs.Update(os.GetDeltaTime());
		cam_mover.Update(0.0);
		os.OSMessageLoop();
		os.SwapBuffers();
	}

	return 0;
}
