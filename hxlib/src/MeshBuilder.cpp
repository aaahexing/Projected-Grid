#include "MeshBuilder.h"

#include "TopologyHandler.h"

MeshBuilder::MeshBuilder() {
}

int MeshBuilder::addPosition(const glm::vec3 &_pos) {
	int old_sz = (int)m_positions.size();
	m_positions.push_back(_pos);
	return old_sz;
}

int MeshBuilder::addNormal(const glm::vec3 &_nor) {
	int old_sz = (int)m_normals.size();
	m_normals.push_back(_nor);
	return old_sz;
}

int MeshBuilder::addTexcoord(const glm::vec2 &_tex) {
	int old_sz = (int)m_texcoords.size();
	m_texcoords.push_back(_tex);
	return old_sz;
}

void MeshBuilder::addFace(const std::vector<int> &_face) {
	int n = _face.size();
	m_face_n.push_back(n);
	m_face_indices[POSITION_TOP].insert(m_face_indices[POSITION_TOP].end(), _face.begin(), _face.end());
}

void MeshBuilder::addFace(const std::vector<int> &_face, const std::vector<int> &_face_tex) {
	int n = _face.size();
	m_face_n.push_back(n);
	m_face_indices[POSITION_TOP].insert(m_face_indices[POSITION_TOP].end(), _face.begin(), _face.end());
	m_face_indices[TEXCOORD_TOP].insert(m_face_indices[TEXCOORD_TOP].end(), _face_tex.begin(), _face_tex.end());
}

void MeshBuilder::startMesh() {
	m_face_num = 0;
	m_vertex_num = 0;
	m_vertex_num_tex = 0;
	m_face_n.clear();
	m_face_off.clear();
	m_face_indices[POSITION_TOP].clear();
	m_face_indices[TEXCOORD_TOP].clear();
	m_normals.clear();
	m_positions.clear();
	m_texcoords.clear();
}

void MeshBuilder::finishMesh() {
	m_face_num = (int)m_face_n.size();
	m_vertex_num = (int)m_positions.size();
	m_vertex_num_tex = (int)m_texcoords.size();
	// Precompute the offset array for faces
	int offset = 0;
	m_face_off.resize(m_face_num);
	for (int f = 0; f < m_face_num; ++f) {
		const int n = m_face_n[f];
		m_face_off[f] = offset;
		offset += n;
	}
}

void MeshBuilder::recomputeNormals() {
	const std::vector<int> &face_indices = m_face_indices[POSITION_TOP];
	m_normals = std::vector<glm::vec3>(m_vertex_num, glm::vec3(0.f));
	for (int f = 0; f < m_face_num; ++f) {
		const int n = m_face_n[f];
		const int *face = &face_indices[m_face_off[f]];
		glm::vec3 face_normal(0.f, 0.f, 0.f);
		for (int i = 0; i < n; ++i) {
			const glm::vec3 &p0 = m_positions[face[(i+n-1)%n]];
			const glm::vec3 &p1 = m_positions[face[i]];
			const glm::vec3 &p2 = m_positions[face[(i+1)%n]];
			face_normal += glm::cross(p2 - p1, p0 - p1);
		}
		for (int i = 0; i < n; ++i)
			m_normals[face[i]] += face_normal;
	}
	for (int v = 0; v < m_vertex_num; ++v)
		m_normals[v] = glm::normalize(m_normals[v]);
}

