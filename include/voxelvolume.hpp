#pragma once

#include <unordered_map>
#include <map>
#include <vector>
#include <queue>
#include <cstdint>

#include "command-queue.hpp"

namespace vv {
	struct Vertex;

	struct Voxel {
		enum NEIGHBORS { UP = 0, DOWN, LEFT, RIGHT, FRONT, BACK };
		float color[3];
		std::weak_ptr<Voxel> neighbors[6];
	};

	enum VOXEL_COMMAND { VOXEL_ADD, VOXEL_REMOVE };

	struct VoxelCommand : Command < VOXEL_COMMAND > {
		VoxelCommand(const VOXEL_COMMAND voxel_c, const GUID entity_id,
			std::shared_ptr<CallbackHolder> callback = nullptr,
			std::tuple<std::int16_t, std::int16_t, std::int16_t> position = std::make_tuple(0, 0, 0)) :
			Command(voxel_c, entity_id, callback) {
			this->row = std::get<0>(position);
			this->column = std::get<1>(position);
			this->slice = std::get<2>(position);
		}
		std::int16_t row, column, slice;
	};

	class VoxelVolume : public CommandQueue < VOXEL_COMMAND > {
	public:
		VoxelVolume();
		~VoxelVolume();

	protected:
		// All values are relative to the front orientation and centered on the root voxel.
		// Slice is depth (away from the screen is positive). Row is up/down. Column is left/right.
		// Therefore adding a voxel to the front face is AddVoxel(0, 0, 1).
		void AddVoxel(const std::int16_t row, const std::int16_t column, const std::int16_t slice);

		// See AddVoxel().
		void RemoveVoxel(const std::int16_t row, const std::int16_t column, const std::int16_t slice);

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
		std::unordered_map<long long, std::shared_ptr<Voxel>> voxels;
		std::vector<Vertex> verts;
		std::vector<unsigned int> indicies;
		std::map<std::tuple<float, float, float>, unsigned int> index_list;
	};
}
