#pragma once

#include <vector>
#include <memory>
#include <map>
#include "vertexbuffer.hpp"

namespace vv {
	class Material;

	class PolygonMeshData {
	public:
		void SetMaterial(std::weak_ptr<Material> m, size_t submesh = 0) {
			this->materials[submesh] = m;
		}

		// Returns the Material for specified submesh.
		std::weak_ptr<Material> GetMaterial(size_t submesh = 0) {
			if (this->materials.find(submesh) != this->materials.end()) {
				return this->materials[submesh];
			}
		}

		void AddVertex(Vertex v, size_t submesh = 0) {
			this->verts[submesh].push_back(v);
		}

		// Returns the vertex buffer for specified submesh.
		const std::vector<Vertex>& GetVertexBuffer(size_t submesh = 0) {
			if (this->verts.find(submesh) != this->verts.end()) {
				return this->verts[submesh];
			}
		}

		void AddIndex(unsigned int i, size_t submesh = 0) {
			this->indicies[submesh].push_back(i);
		}

		// Returns the index buffer for specified submesh.
		const std::vector<unsigned int>& GetIndexBuffer(size_t submesh = 0) {
			if (this->indicies.find(submesh) != this->indicies.end()) {
				return this->indicies[submesh];
			}
		}
	private:
		std::map<size_t, std::weak_ptr<Material>> materials;
		std::map<size_t, std::vector<Vertex>> verts;
		std::map<size_t, std::vector<unsigned int>> indicies;
	};
}
