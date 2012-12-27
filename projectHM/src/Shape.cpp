#include "projectHM_PCH.h"

#include "Shape.h"

Line::Line(const glm::vec3 &_a, const glm::vec3 &_b) {
	a = _a, b = _b;
}

Plane::Plane(const float *_p) {
	a = _p[0], b = _p[1], c = _p[2], d = _p[3];
}

Plane::Plane(const Plane &_p) {
	a = _p.a, b = _p.b, c = _p.c, d = _p.d;
}

Plane::Plane(const glm::vec3 &_p, const glm::vec3 &_n) {
	a = _n.x, b = _n.y, c = _n.z, d = -(_p.x * _n.x + _p.y * _n.y + _p.z * _n.z);
}

Plane::Plane(float _a, float _b, float _c, float _d) {
	a = _a, b = _b, c = _c, d = _d;
}
