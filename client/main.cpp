#include "os.hpp"
#include "event-system.hpp"
#include "event-queue.hpp"
#include "render-system.hpp"
#include "vertexbuffer.hpp"
#include "shader.hpp"
#include "voxelvolume.hpp"
#include "transform.hpp"
#include "material.hpp"
#include "entity.hpp"
#include "components/camera.hpp"
#include "polygonmeshdata.hpp"
#include "component-update-system.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <map>

#include <asio.hpp>
#include <array>
#include "message.hpp"
#include "position-message.hpp"
#include "client.hpp"
#include <thread>

vv::eid player_id;
std::string username;

std::list<std::function<void(vv::frame_id_t)>> vv::ComponentUpdateSystemList::update_funcs;

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

	vv::ComponentUpdateSystem<vv::Position>::Initialize();
	vv::ComponentUpdateSystem<vv::Orientation>::Initialize();
	vv::ComponentUpdateSystem<vv::Camera>::Initialize();
	vv::ComponentUpdateSystem<vv::Renderable>::Initialize();
	vv::ComponentUpdateSystem<vv::View>::Initialize();

	vv::Entity voxel1(100);
	voxel1.Add<vv::Position>();
	voxel1.Add<vv::Orientation>();
	voxel1.Add<vv::Renderable>();

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

	vv::Entity camera(1);
	camera.Add<vv::Position>();
	camera.Add<vv::Orientation>();
	camera.Add<vv::Camera>(1);
	camera.Add<vv::Renderable>();
	vv::Entity camera2(2);
	camera2.Add<vv::Position>();
	camera2.Add<vv::Orientation>();
	camera2.Add<vv::Camera>(2);

	vv::CameraMover cam_mover(1);

	asio::io_service io_service;

	tcp::resolver resolver(io_service);
	tcp::resolver::iterator endpoint_iterator = resolver.resolve({"localhost", "12345"});

	chat_client c(io_service, endpoint_iterator);
	std::thread t([&io_service] () { io_service.run(); });
	auto chat_listen = [&c] () {
		std::cout << "username:";
		std::cin >> username;
		std::cout << "id:";
		std::cin >> player_id;

		std::cin.clear();
		std::cin.ignore(10000, '\n');

		char line[chat_message::max_body_length + 1];
		while (std::cin.getline(line, chat_message::max_body_length + 1)) {
			chat_message msg;
			std::string str;
			str += username;
			str += ": ";
			str += line;
			msg.body_length(str.size());
			std::memcpy(msg.body(), str.c_str(), msg.body_length());
			msg.encode_header();
			c.write(msg);
		}
	};
	std::thread t2(chat_listen);

	std::int64_t frame_id = 1;

	while (!os.Closing()) {
		vv::ComponentUpdateSystemList::UpdateAll(frame_id);

		cam_mover.Update(0.0);
		rs.Update(os.GetDeltaTime());
		os.OSMessageLoop();
		os.SwapBuffers();

		vv::position_message pm;
		for (auto itr = vv::Multiton<vv::eid, std::shared_ptr<vv::Position>>::Begin(); itr != vv::Multiton<vv::eid, std::shared_ptr<vv::Position>>::End(); ++itr) {
			auto entity_id = itr->first;
			pm.FrameID(frame_id);
			auto position = itr->second->value;
			std::stringstream ss;
			ss << position.x << "," << position.y << "," << position.z;
			/*glm::vec3 test;
			char c;
			ss >> test.x >> c >> test.y >> c >> test.z;*/
			std::memcpy(pm.body(), ss.str().c_str(), ss.str().size());
			pm.body_length(ss.str().size());
			pm.encode_header();
			c.write(pm);
		}
		frame_id++;
	}
	c.close();
	t.join();
	t2.join();

	return 0;
}
