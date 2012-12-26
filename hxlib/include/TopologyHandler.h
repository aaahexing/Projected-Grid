#ifndef __TOPOLOGYHANDLER_H__
#define __TOPOLOGYHANDLER_H__

#include "HXLib.h"

class TopologyHandler {
public:
	TopologyHandler() {
		m_face_degrees = NULL;
		m_face_indices = NULL;
	}

	struct EdgeNode {
		int a, b, mask;	// face_i * 4 + inface_i
	public:
		bool operator < (const EdgeNode &e) const {
			return a < e.a || (a == e.a && b < e.b);
		}
		bool operator == (const EdgeNode &e) const {
			return a == e.a && b == e.b;
		}
		bool operator != (const EdgeNode &e) const {
			return a != e.a || b != e.b;
		}
	};

	struct EdgeNeib {
		int a, mask;
	public:
		bool operator < (const EdgeNeib &e) const {
			return a < e.a || (a == e.a && mask < e.mask);
		}
		bool operator == (const EdgeNeib &e) const {
			return mask == e.mask;
		}
	};

	// edge mask handler
	inline int getEdgeMask(int face_i, int i) const {
		return (face_i << 2) | i;
	}
	inline int getEdgeFaceI(int mask) const {
		return mask >> 2;
	}
	inline int getEdgeInFaceI(int mask) const {
		return mask & 3;
	}
	inline int getPrevEdgeMask(int mask) const {
		int n = m_face_degrees[getEdgeFaceI(mask)];
		return (mask & (~3)) | ((getEdgeInFaceI(mask) + n - 1) % n);
	}
	inline int getNextEdgeMask(int mask) const {
		int n = m_face_degrees[getEdgeFaceI(mask)];
		return (mask & (~3)) | ((getEdgeInFaceI(mask) + 1) % n);
	}
	inline int getShiftedMask(int mask, int i) const {
		int n = m_face_degrees[getEdgeFaceI(mask)];
		return (mask & (~3)) | ((getEdgeInFaceI(mask) + i) % n);
	}
	inline int getEdgeAdjacentMask(int mask) const {
		assert(m_edge_adj[mask] >= 0);
		return m_edge_adj[mask];
	}
	inline int getEdgeSource(int mask) const {
		return m_face_indices[m_face_offset[mask >> 2] + (mask & 3)];
	}
	inline int getEdgeTarget(int mask) const {
		return getEdgeSource(getNextEdgeMask(mask));
	}
	// check whether the edge is sharp
	inline int isEdgeInfiniteSharp(int mask) const {
		return m_edge_adj[mask] == -1;
	}

	// crucial methods
	void buildEdgeMap();
	void setIndexArray(const int _face_num, const int _vertex_num, const int *_face_degrees, const int *_face_indices);

protected:
	int m_face_num;
	int *m_face_degrees;
	int *m_face_indices;
	std::vector<int> m_edge_adj;
	std::vector<int> m_face_offset;
	std::vector<EdgeNode> edge_set;
};

#endif	// __TOPOLOGYHANDLER_H__