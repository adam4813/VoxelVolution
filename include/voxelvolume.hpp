#pragma once

#include <unordered_map>
#include <map>
#include <memory>
#include <vector>
#include <queue>
#include <cstdint>

#include "command-queue.hpp"

namespace vv {
	struct Vertex;
	class PolygonMeshData;

	struct Voxel {
		enum NEIGHBORS { UP = 0, DOWN, LEFT, RIGHT, FRONT, BACK };
		float color[3];
		std::weak_ptr<Voxel> neighbors[6];
	};

	enum VOXEL_COMMAND { VOXEL_ADD, VOXEL_REMOVE };

	struct VoxelCommand : Command < VOXEL_COMMAND > {
		VoxelCommand(const VOXEL_COMMAND voxel_c, const GUID entity_id,
			std::shared_ptr<Callback> callback = nullptr,
			std::tuple<std::int16_t, std::int16_t, std::int16_t> position = std::make_tuple(0, 0, 0)) :
			Command(voxel_c, entity_id, callback) {
			this->row = std::get<0>(position);
			this->column = std::get<1>(position);
			this->slice = std::get<2>(position);
		}
		std::int16_t row, column, slice;
	};

	class VoxelVolume;
	typedef Multiton<GUID, std::shared_ptr<VoxelVolume>> VoxelVoumeMap;

	class VoxelVolume : public CommandQueue < VOXEL_COMMAND > {
	public:
		VoxelVolume(const GUID entity_id, std::weak_ptr<PolygonMeshData> mesh, const size_t submesh = 0);
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
		void UpdateMesh();

		std::weak_ptr<PolygonMeshData> GetMesh();

		// Creates a VoxelVolume for entity_id and uses a PolygonMeshData with name and into submesh.
		static std::weak_ptr<VoxelVolume> Create(const GUID entity_id,
			const std::string name, const size_t submesh = 0);
		// Creates a VoxelVolume for entity_id and uses PolygonMeshData and into submesh.
		static std::weak_ptr<VoxelVolume> Create(const GUID entity_id,
			std::weak_ptr<PolygonMeshData> mesh = std::weak_ptr<PolygonMeshData>(), const size_t submesh = 0);
	private:
		std::unordered_map<std::int64_t, std::shared_ptr<Voxel>> voxels;
		std::weak_ptr<PolygonMeshData> mesh;
		size_t submesh;
		GUID entity_id;
	};
}
