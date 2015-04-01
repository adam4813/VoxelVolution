#include "voxelvolume.hpp"
#include "vertexbuffer.hpp"

namespace vv {
    std::atomic<std::queue<std::shared_ptr<Command<VOXEL_COMMAND>>>*> VoxelVolume::global_queue = new std::queue<std::shared_ptr<Command<VOXEL_COMMAND>>>();

	VoxelVolume::VoxelVolume() { }

	VoxelVolume::~VoxelVolume() { }

	void VoxelVolume::AddVoxel(const std::int16_t row, const std::int16_t column, const std::int16_t slice) {
		Voxel v;
		int64_t index = (static_cast<std::uint64_t>(row & 0xFFFF) << 32) + (static_cast<std::uint32_t>(column & 0xFFFF) << 16) + static_cast<std::uint16_t>(slice & 0xFFFF);

		if (this->voxels.find(index) == this->voxels.end()) {
			this->voxels[index] = std::make_shared<Voxel>();

			// Since we are adding a voxel we must set the new voxels neighbors.
			std::int64_t up_index, down_index, left_index, right_index, back_index, front_index;
			up_index = (static_cast<std::uint64_t>((row + 1) & 0xFFFF) << 32) + (static_cast<std::uint32_t>(column & 0xFFFF) << 16) + static_cast<std::uint16_t>(slice & 0xFFFF);
			down_index = (static_cast<std::uint64_t>((row - 1) & 0xFFFF) << 32) + (static_cast<std::uint32_t>(column & 0xFFFF) << 16) + static_cast<std::uint16_t>(slice & 0xFFFF);
			left_index = (static_cast<std::uint64_t>(row & 0xFFFF) << 32) + (static_cast<std::uint32_t>((column - 1) & 0xFFFF) << 16) + static_cast<std::uint16_t>(slice & 0xFFFF);
			right_index = (static_cast<std::uint64_t>(row & 0xFFFF) << 32) + (static_cast<std::uint32_t>((column + 1) & 0xFFFF) << 16) + static_cast<std::uint16_t>(slice & 0xFFFF);
			front_index = (static_cast<std::uint64_t>(row & 0xFFFF) << 32) + (static_cast<std::uint32_t>(column & 0xFFFF) << 16) + static_cast<std::uint16_t>((slice - 1) & 0xFFFF);
			back_index = (static_cast<std::uint64_t>(row & 0xFFFF) << 32) + (static_cast<std::uint32_t>(column & 0xFFFF) << 16) + static_cast<std::uint16_t>((slice + 1) & 0xFFFF);

			if (this->voxels.find(up_index) != this->voxels.end()) {
				v.neighbors[Voxel::UP] = this->voxels[up_index];
				this->voxels[up_index]->neighbors[Voxel::DOWN] = this->voxels[index];
			}
			if (this->voxels.find(down_index) != this->voxels.end()) {
				v.neighbors[Voxel::DOWN] = this->voxels[down_index];
				this->voxels[down_index]->neighbors[Voxel::UP] = this->voxels[index];
			}
			if (this->voxels.find(left_index) != this->voxels.end()) {
				v.neighbors[Voxel::LEFT] = this->voxels[left_index];
				this->voxels[left_index]->neighbors[Voxel::RIGHT] = this->voxels[index];
			}
			if (this->voxels.find(right_index) != this->voxels.end()) {
				v.neighbors[Voxel::RIGHT] = this->voxels[right_index];
				this->voxels[right_index]->neighbors[Voxel::LEFT] = this->voxels[index];
			}
			if (this->voxels.find(front_index) != this->voxels.end()) {
				v.neighbors[Voxel::FRONT] = this->voxels[front_index];
				this->voxels[front_index]->neighbors[Voxel::BACK] = this->voxels[index];
			}
			if (this->voxels.find(back_index) != this->voxels.end()) {
				v.neighbors[Voxel::BACK] = this->voxels[back_index];
				this->voxels[back_index]->neighbors[Voxel::FRONT] = this->voxels[index];
			}
		}
	}

	void VoxelVolume::RemoveVoxel(const std::int16_t row, const std::int16_t column, const std::int16_t slice) {
		int64_t index = (static_cast<std::uint64_t>(row & 0xFFFF) << 32) + (static_cast<std::uint32_t>(column & 0xFFFF) << 16) + static_cast<std::uint16_t>(slice & 0xFFFF);

		if (this->voxels.find(index) != this->voxels.end()) {
			std::weak_ptr<Voxel> v = this->voxels[index];
			this->voxels.erase(index);
		}
	}

	void VoxelVolume::Update(double delta) {
		ProcessCommandQueue();
		UpdateVertexBuffers();
	}

	void VoxelVolume::ProcessCommandQueue() {
		this->local_queue = global_queue.exchange(this->local_queue);

		while (!local_queue->empty()) {
			auto action = local_queue->front();
			local_queue->pop();
			auto voxel_action = std::static_pointer_cast<VoxelCommand>(action);

			switch (action->command) {
				case VOXEL_ADD:
					AddVoxel(voxel_action->row, voxel_action->column, voxel_action->slice);
					break;
				case VOXEL_REMOVE:
					RemoveVoxel(voxel_action->row, voxel_action->column, voxel_action->slice);
					break;
			}
		}
	}

	void VoxelVolume::UpdateVertexBuffers() {
		this->verts.clear();
		this->index_list.clear();
		this->indicies.clear();
		static std::vector<Vertex> IdentityVerts({
			// Front
			Vertex(-1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f),	// Bottom left
			Vertex(1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 0.0f),	// Bottom right
			Vertex(1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f),		// Top right
			Vertex(-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f),	// Top Left
			// Back
			Vertex(-1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f),	// Bottom left
			Vertex(1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f),	// Bottom right
			Vertex(1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f),	// Top right
			Vertex(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f)	// Top left
		});

		for (auto v : this->voxels) {
			std::int16_t row = static_cast<std::int16_t>((v.first & 0xFFFF00000000) >> 32);
			std::int16_t column = static_cast<std::int16_t>((v.first & 0xFFFF0000) >> 16);
			std::int16_t slice = static_cast<std::int16_t>(v.first & 0xFFFF);
			GLuint index[8];

			for (size_t i = 0; i < 8; ++i) {
				auto vert_position = std::make_tuple(IdentityVerts[i].position[0] + column * 2,
					IdentityVerts[i].position[1] + row * 2, IdentityVerts[i].position[2] + slice * 2);

				if (this->index_list.find(vert_position) == this->index_list.end()) {
					this->verts.push_back(Vertex(IdentityVerts[i].position[0] + column * 2,
						IdentityVerts[i].position[1] + row * 2, IdentityVerts[i].position[2] + slice * 2,
						IdentityVerts[i].color[0], IdentityVerts[i].color[1], IdentityVerts[i].color[2]));
					//v.second.color[0], v.second.color[1], v.second.color[2]));
					this->index_list[vert_position] = this->index_list.size();
				}
				index[i] = this->index_list[vert_position];
			}

			// Front
			this->indicies.push_back(index[0]); this->indicies.push_back(index[1]); this->indicies.push_back(index[2]);
			this->indicies.push_back(index[2]); this->indicies.push_back(index[3]); this->indicies.push_back(index[0]);
			// Top
			this->indicies.push_back(index[3]); this->indicies.push_back(index[2]); this->indicies.push_back(index[6]);
			this->indicies.push_back(index[6]); this->indicies.push_back(index[7]); this->indicies.push_back(index[3]);
			// Back
			this->indicies.push_back(index[7]); this->indicies.push_back(index[6]); this->indicies.push_back(index[5]);
			this->indicies.push_back(index[5]); this->indicies.push_back(index[4]); this->indicies.push_back(index[7]);
			// Bottom
			this->indicies.push_back(index[4]); this->indicies.push_back(index[5]); this->indicies.push_back(index[1]);
			this->indicies.push_back(index[1]); this->indicies.push_back(index[0]); this->indicies.push_back(index[4]);
			// Left
			this->indicies.push_back(index[4]); this->indicies.push_back(index[0]); this->indicies.push_back(index[3]);
			this->indicies.push_back(index[3]); this->indicies.push_back(index[7]); this->indicies.push_back(index[4]);
			// Right
			this->indicies.push_back(index[1]); this->indicies.push_back(index[5]); this->indicies.push_back(index[6]);
			this->indicies.push_back(index[6]); this->indicies.push_back(index[2]); this->indicies.push_back(index[1]);
		}
	}
}
