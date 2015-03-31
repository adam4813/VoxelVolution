#pragma once

#include <unordered_map>
#include <map>
#include <vector>
#include <queue>

#include "command-queue.hpp"

namespace vv {
	struct Vertex;

	struct Voxel {
		Voxel() {
			this->neighbors[UP] = nullptr; this->neighbors[DOWN] = nullptr;
			this->neighbors[LEFT] = nullptr; this->neighbors[RIGHT] = nullptr;
			this->neighbors[FRONT] = nullptr; this->neighbors[BACK] = nullptr;
		}
		enum NEIGHBORS { UP = 0, DOWN, LEFT, RIGHT, FRONT, BACK };
		float color[3];
		Voxel* neighbors[6];
	};

	enum VOXEL_COMMAND { VOXEL_ADD, VOXEL_REMOVE };

	struct VoxelCommand : Command < VOXEL_COMMAND > {
		VoxelCommand(const VOXEL_COMMAND voxel_c, const GUID entity_id, std::tuple<short, short, short> position) :
			Command(voxel_c, entity_id) {
			this->row = std::get<0>(position);
			this->column = std::get<1>(position);
			this->slice = std::get<2>(position);
		}
		short row, column, slice;
	};

	class VoxelVolume : public CommandQueue < VOXEL_COMMAND > {
	public:
		VoxelVolume();
		~VoxelVolume();

	protected:
		// All values are relative to the front orientation and centered on the root voxel.
		// Slice is depth (away from the screen is positive). Row is up/down. Column is left/right.
		// Therefore adding a voxel to the front face is AddVoxel(0, 0, 1).
		void AddVoxel(const short row, const short column, const short slice);

		// See AddVoxel().
		void RemoveVoxel(const short row, const short column, const short slice);

		void ProcessCommandQueue();
	public:
		// Iterates over all the actions queued before the call to update.
		void Update(double delta);

		// Generates a vertex (and index) buffer for the current voxel state.
		void UpdateVertexBuffers();

		// Returns the vertex buffer.
		const std::vector<Vertex>& GetVertexBuffer() {
			return this->verts;
		}

		// Returns the index buffer.
		const std::vector<unsigned int>& GetIndexBuffer() {
			return this->indicies;
		}
	private:
		std::unordered_map<long long, Voxel> voxels;
		std::vector<Vertex> verts;
		std::vector<unsigned int> indicies;
		std::map<std::tuple<float, float, float>, unsigned int> index_list;
	};
}
