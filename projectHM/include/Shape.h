#ifndef __SHAPE_H__
#define __SHAPE_H__

struct Line {
	glm::vec3 a;
	glm::vec3 b;
public:
	Line() {}
	Line(const glm::vec3 &_a, const glm::vec3 &_b);
};

struct Plane {
	float a, b, c, d;
public:
	Plane() {}
	Plane(const float *_p);
	Plane(const Plane &_p);
	Plane(const glm::vec3 &_p, const glm::vec3 &_n);
	Plane(float _a, float _b, float _c, float _d);

	inline glm::vec3 getNormal() const {
		return glm::vec3(a, b, c);
	}
};

const float EPS = 1e-8;

inline bool zero(double x) {
	return (x > 0 ? x : -x) < EPS;
}

// Cross product of two vectors
inline glm::vec3 xmult(const glm::vec3 &u, const glm::vec3 &v) {
	glm::vec3 ret;
	ret.x = u.y * v.z - v.y * u.z;
	ret.y = u.z * v.x - u.x * v.z;
	ret.z = u.x * v.y - u.y * v.x;
	return ret;
}

// Dot product of two vectors
inline glm::vec3 subt(const glm::vec3 &u, const glm::vec3 &v) {
	glm::vec3 ret;
	ret.x = u.x - v.x;
	ret.y = u.y - v.y;
	ret.z = u.z - v.z;
	return ret;
}

// Vector substitution
inline float dmult(const glm::vec3 &u, const glm::vec3 &v) {
	return u.x * v.x + u.y * v.y + u.z * v.z;
}

// Get normal of a plane
inline glm::vec3 pvec(const Plane &p) {
	return glm::vec3(p.a, p.b, p.c);
}

// Check whether a line is parallel to a plane
inline bool parallel(const Line & l, const Plane & p) {
	return zero(dmult(subt(l.a, l.b), pvec(p)));
}

// Get the intersection between a line and a plane
inline glm::vec3 intersection(const Line &l, const Plane &p) {
	glm::vec3 ret = subt(l.b, l.a);
	double t = -(dmult(pvec(p), l.a) + p.d) / (dmult(pvec(p), ret));
	ret.x = ret.x * t + l.a.x;
	ret.y = ret.y * t + l.a.y;
	ret.z = ret.z * t + l.a.z;
	return ret;
}

// Get the distance from a point to a plane
inline float distance(const glm::vec3 &p, const Plane &pl) {
	return p.x * pl.a + p.y * pl.b + p.z * pl.c;
}

// a*x + b*y + c*z + d*0
// Calculating the angle between the normal of the plane, and another normal
inline float planeDotNormal(const Plane &p, const glm::vec3 &d) {
	return p.a * d.x + p.b * d.y + p.c * d.z;
}

// a*x + b*y + c*z + d*1
// Determining the plane's relationship with a coordinate in 3D space
inline float planeDotCoord(const Plane &p, const glm::vec3 &d) {
	return p.a * d.x + p.b * d.y + p.c * d.z + p.d * 1.f;
}

#endif	/* __SHAPE_H__ */