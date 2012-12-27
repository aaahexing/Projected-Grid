#ifndef __TRANSFORM_H__
#define __TRANSFORM_H__

inline glm::vec3 transformPoint(float *m_transform, const glm::vec3 &p) {
	float x = (m_transform[0] * p.x + m_transform[4] * p.y
		+ m_transform[8] * p.z + m_transform[12]);
	float y = (m_transform[1] * p.x + m_transform[5] * p.y
		+ m_transform[9] * p.z + m_transform[13]);
	float z = (m_transform[2] * p.x + m_transform[6] * p.y
		+ m_transform[10] * p.z + m_transform[14]);
	float w = (m_transform[3] * p.x + m_transform[7] * p.y
		+ m_transform[11] * p.z + m_transform[15]);
	if (w == 1.f)
		return glm::vec3(x, y, z);
	else
		return glm::vec3(x, y, z) / w;
}

inline glm::vec3 transformPoint(const glm::mat4 m_transform, const glm::vec3 &p) {
	float x = m_transform[0][0] * p.x + m_transform[1][0] * p.y
		+ m_transform[2][0] * p.z + m_transform[3][0];
	float y = m_transform[0][1] * p.x + m_transform[1][1] * p.y
		+ m_transform[2][1] * p.z + m_transform[3][1];
	float z = m_transform[0][2] * p.x + m_transform[1][2] * p.y
		+ m_transform[2][2] * p.z + m_transform[3][2];
	float w = m_transform[0][3] * p.x + m_transform[1][3] * p.y
		+ m_transform[2][3] * p.z + m_transform[3][3];
#ifdef projectHM_DEBUG
	if (w == 0.f) {
		fprintf(stderr, "w == 0 in Transform::transformPoint\n");
	}
#endif
	if (w == 1.f)
		return glm::vec3(x, y, z);
	else
		return glm::vec3(x, y, z) / w;
}

inline glm::vec4 transformPoint(const glm::mat4 m_transform, const glm::vec4 &p) {
	return m_transform * p;
}

inline glm::vec3 transformAffinePoint(const glm::mat4 m_transform, const glm::vec3 &p) {
	float x = m_transform[0][0] * p.x + m_transform[1][0] * p.y
		+ m_transform[2][0] * p.z + m_transform[3][0];
	float y = m_transform[0][1] * p.x + m_transform[1][1] * p.y
		+ m_transform[2][1] * p.z + m_transform[3][1];
	float z = m_transform[0][2] * p.x + m_transform[1][2] * p.y
		+ m_transform[2][2] * p.z + m_transform[3][2];
	return glm::vec3(x, y, z);
}

inline glm::vec3 transformVector(const glm::mat4 m_transform, const glm::vec3 &v) {
	float x = m_transform[0][0] * v.x + m_transform[1][0] * v.y
		+ m_transform[2][0] * v.z;
	float y = m_transform[0][1] * v.x + m_transform[1][1] * v.y
		+ m_transform[2][1] * v.z;
	float z = m_transform[0][2] * v.x + m_transform[1][2] * v.y
		+ m_transform[2][2] * v.z;
	return glm::vec3(x, y, z);
}


#endif