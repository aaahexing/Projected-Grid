#ifndef __MESHBUILDER_H__
#define __MESHBUILDER_H__

#include "HXLib.h"

class MeshBuilder {
public:
	enum TopologyType {
		POSITION_TOP = 0,
		TEXCOORD_TOP = 1
	};

	MeshBuilder();

	inline int getFaceNum() const {
		return m_face_num;
	}
	inline int getVertexNum() const {
		return m_vertex_num;
	}
	inline int getVertexNumTex() const {
		return m_vertex_num_tex;
	}
	inline const glm::vec3& getVertexNormal(int vi) const {
		assert(vi < m_vertex_num);
		return m_normals[vi];
	}
	inline const glm::vec3& getVertexPosition(int vi) const {
		assert(vi < m_vertex_num);
		return m_positions[vi];
	}
	inline const glm::vec2& getVertexTexcoord(int vi) const {
		assert(vi < m_vertex_num_tex);
		return m_texcoords[vi];
	}
	inline bool hasTexcoords() const {
		return m_vertex_num_tex > 0;
	}
	inline bool hasNormals() const {
		return !m_normals.empty();
	}

	// Add geometry data
	int addPosition(const glm::vec3 &_pos);
	int addNormal(const glm::vec3 &_nor);
	int addTexcoord(const glm::vec2 &_tex);
	// Add topology data
	void addFace(const std::vector<int> &_face);
	void addFace(const std::vector<int> &_face, const std::vector<int> &_face_tex);
	// Initialize all the needed data
	void startMesh();
	// Do some post-process of the mesh
	void finishMesh();

	// Compute normal (@todo: use crease angle method)
	void recomputeNormals();

protected:
	int m_face_num;
	int m_vertex_num;
	int m_vertex_num_tex;
	std::vector<int> m_face_n;
	std::vector<int> m_face_off;
	std::vector<int> m_face_indices[2];
	std::vector<glm::vec3> m_normals;
	std::vector<glm::vec3> m_positions;
	std::vector<glm::vec2> m_texcoords;
};

#endif
