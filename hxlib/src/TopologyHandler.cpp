#include "TopologyHandler.h"

#include <algorithm>

void TopologyHandler::buildEdgeMap() {
	using namespace std;
	// start to build edge map
	int edge_num = m_face_num << 2;
	edge_set.resize(edge_num);
	m_edge_adj.resize(edge_num);
	for (int e = 0; e < edge_num; ++e) {
		edge_set[e].a = edge_set[e].b = edge_set[e].mask = 0x7fffffff;
		m_edge_adj[e] = -1;
	}
	for (int f = 0; f < m_face_num; ++f) {
		const int n = m_face_degrees[f];
		const int *face = &m_face_indices[m_face_offset[f]];
		for (int i = 0; i < n; ++i) {
			int mask = getEdgeMask(f, i);
			edge_set[mask].mask = mask;
			edge_set[mask].a = face[i];
			edge_set[mask].b = face[(i + 1) % n];
			if (edge_set[mask].a > edge_set[mask].b)
				swap(edge_set[mask].a, edge_set[mask].b);
		}
	}
	sort(edge_set.begin(), edge_set.end());
	for (int e = 0; e < edge_num; ++e) {
		if (edge_set[e].a >= 0x7fffffff) {
			edge_num = e;
			break;
		}
	}
	for (int e = 0; e < edge_num; ++e) {
		if (e > 0 && edge_set[e] == edge_set[e - 1])
			m_edge_adj[edge_set[e].mask] = edge_set[e - 1].mask;
		else if (e + 1 < edge_num && edge_set[e] == edge_set[e + 1])
			m_edge_adj[edge_set[e].mask] = edge_set[e + 1].mask;
		else
			m_edge_adj[edge_set[e].mask] = -1;		// boundary edge
	}
	// edges shared by more than 2 faces are crease edges
	for (int e = 0; e < edge_num; ++e) {
		if (!e || edge_set[e] != edge_set[e - 1]) {
			int e_end = e + 1;
			for (; e_end < edge_num && edge_set[e_end] == edge_set[e]; ++e_end);
			if (e_end > e + 2) {
				// non-manifold edge
				for (int i = e; i < e_end; ++i)
					m_edge_adj[edge_set[i].mask] = -1;
			}
		}
	}
	// seems to be no use after the `edge_map' has been built up
	edge_set.clear();
}

void TopologyHandler::setIndexArray(const int _face_num, const int _vertex_num, const int *_face_degrees, const int *_face_indices) {
	m_face_num = _face_num;
	m_face_degrees = const_cast<int*>(_face_degrees);
	m_face_indices = const_cast<int*>(_face_indices);
	m_face_offset.resize(m_face_num);
	int offset = 0;
	for (int f = 0; f < m_face_num; ++f) {
		m_face_offset[f] = offset;
		offset += m_face_degrees[f];
	}
	// build the edge maps
	buildEdgeMap();
}
