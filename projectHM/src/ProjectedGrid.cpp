#include "projectHM_PCH.h"

#include "ProjectedGrid.h"

#include "gl/glew.h"
#include "gl/glut.h"

#include "Shape.h"
#include "Camera.h"
#include "Transform.h"

ProjectedGrid::ProjectedGrid(const Plane &base_plane, const Camera *camera, const ProjectedGridOptions &options)
	: m_base_plane(base_plane), m_projecting_camera(NULL), m_rendering_camera(camera) {
	// @hack: need to calculate the real bound
	m_upper_bound_plane = base_plane;
	m_lower_bound_plane = base_plane;
	setOptions(options);
}

ProjectedGrid::~ProjectedGrid() {
	if (m_projecting_camera) {
		delete m_projecting_camera;
		m_projecting_camera = NULL;
	}
}

void ProjectedGrid::setOptions(const ProjectedGridOptions &options) {
	m_options = options;
	m_vertices.resize(options.sides * options.sides);
}

bool ProjectedGrid::getRangeMatrix(float water_max_height, float water_min_height, float projector_height_inc) {
	glm::mat4 rendering_vp_mat = m_rendering_camera->getViewProjectionMatrix();
	glm::mat4 rendering_vp_mat_inv = glm::inverse(rendering_vp_mat);
	// Get the corners of the view frustum in world-space
	const unsigned NUM_FRUSTUM_PTS = 8;
	glm::vec3 frustum_pts[NUM_FRUSTUM_PTS] = {
		transformPoint(rendering_vp_mat_inv, glm::vec3(-1.f, -1.f, 0.f)),
		transformPoint(rendering_vp_mat_inv, glm::vec3(+1.f, -1.f, 0.f)),
		transformPoint(rendering_vp_mat_inv, glm::vec3(-1.f, +1.f, 0.f)),
		transformPoint(rendering_vp_mat_inv, glm::vec3(+1.f, +1.f, 0.f)),
		transformPoint(rendering_vp_mat_inv, glm::vec3(-1.f, -1.f, 1.f)),
		transformPoint(rendering_vp_mat_inv, glm::vec3(+1.f, -1.f, 1.f)),
		transformPoint(rendering_vp_mat_inv, glm::vec3(-1.f, +1.f, 1.f)),
		transformPoint(rendering_vp_mat_inv, glm::vec3(+1.f, +1.f, 1.f))
	};
	// The edges of the frustum
	const unsigned NUM_EDGES = 12;
	const int edges[NUM_EDGES * 2] = {
		// Near
		0, 1,
		1, 3,
		3, 2,
		2, 0,
		// Mid
		0, 4,
		1, 5,
		3, 7,
		2, 6,
		// Far
		4, 5,
		5, 7,
		7, 6,
		6, 4
	};
	// Get the intersections between the frustum and the two bound planes, visible parts
	std::vector<glm::vec3> intersections;
	for (int ei = 0; ei < NUM_EDGES; ++ei) {
		int src = edges[ei * 2];
		int tar = edges[ei * 2 + 1];
		const glm::vec3 &src_v = frustum_pts[src];
		const glm::vec3 &tar_v = frustum_pts[tar];
		Line line(src_v, tar_v);
		if ((m_upper_bound_plane.a*src_v.x + m_upper_bound_plane.b*src_v.y + m_upper_bound_plane.c*src_v.z + m_upper_bound_plane.d)
			/(m_upper_bound_plane.a*tar_v.x + m_upper_bound_plane.b*tar_v.y + m_upper_bound_plane.c*tar_v.z + m_upper_bound_plane.d) < 0.f) {
				intersections.push_back(intersection(line, m_upper_bound_plane));
		}
		if ((m_lower_bound_plane.a*src_v.x + m_lower_bound_plane.b*src_v.y + m_lower_bound_plane.c*src_v.z + m_lower_bound_plane.d)
			/(m_lower_bound_plane.a*tar_v.x + m_lower_bound_plane.b*tar_v.y + m_lower_bound_plane.c*tar_v.z + m_lower_bound_plane.d) < 0.f) {
				intersections.push_back(intersection(line, m_lower_bound_plane));
		}
	}
	for (int i = 0; i < 8; ++i) {
		const glm::vec3 &p = frustum_pts[i];
		if ((m_upper_bound_plane.a*p.x + m_upper_bound_plane.b*p.y + m_upper_bound_plane.c*p.z + m_upper_bound_plane.d)
			/(m_lower_bound_plane.a*p.x + m_lower_bound_plane.b*p.y + m_lower_bound_plane.c*p.z + m_lower_bound_plane.d) < 0.f) {
				intersections.push_back(p);
		}
	}

	if (intersections.empty()) {
		puts("No intersections!");
		return false;
	} else {
		const glm::vec3 plane_normal = m_base_plane.getNormal();
		const glm::vec3 cam_pos = m_rendering_camera->getPosition();
		const glm::vec3 cam_dir = m_rendering_camera->getDirection();
		// Set the projector
		glm::vec3 projector_pos = cam_pos;
		float height_in_plane = distance(cam_pos, m_base_plane);
		float height_bound = m_options.strength + m_options.elevation;
		bool under_water = height_in_plane < 0.f;
		// If the camera is too close to the upper plane or too low
		if (height_in_plane < height_bound) {
			if (under_water) {
				// Reflected the position
				projector_pos += plane_normal * (height_bound - 2 * height_in_plane);
			} else {
				// Move the position upwards
				projector_pos += plane_normal * (height_bound - height_in_plane);
			}
		}
		glm::vec3 aim_point0, aim_point1;
		// If the camera is aimed away from the plane or located on the plane, simply mirror it's
		//	view-vector against the plane
		if (planeDotCoord(m_base_plane, cam_pos) > 0.f && planeDotNormal(m_base_plane, cam_dir) > 0.f) {
			glm::vec3 flipped_dir = cam_dir - 2.f * plane_normal * glm::dot(plane_normal, cam_dir);
			aim_point0 = intersection(Line(cam_pos, cam_pos + flipped_dir), m_base_plane);
		} else {
			aim_point0 = intersection(Line(cam_pos, cam_pos + cam_dir), m_base_plane);
		}
		// If there's no intersections between the camera's view-vector and the plane,
		//	hack the target vertices
		if (abs(planeDotNormal(m_base_plane, cam_dir)) <= 1e-6) {
			aim_point0 = cam_pos + cam_dir;
		}
		aim_point1 = cam_pos + 10.f * cam_dir;
		aim_point1 = aim_point1 - plane_normal * glm::dot(aim_point1, plane_normal);
		float af = fabs(glm::dot(plane_normal, cam_dir));
		// Fade between aim_point0 & aim_point1 depending on view angle
		glm::vec3 projector_tar = aim_point0 * af + aim_point1 * (1.f - af);
		if (m_projecting_camera) {
			delete m_projecting_camera;
			m_projecting_camera = NULL;
		}
		m_projecting_camera = new Camera(*m_rendering_camera);
		m_projecting_camera->setPosition(projector_pos);
		m_projecting_camera->setDirection(projector_tar - projector_pos);
		///////////////////////////////////////////////////
		// Compute the projector's view projection matrix
		glm::mat4 projector_view_proj_mat = m_projecting_camera->getViewProjectionMatrix();
		// @todo: need definition of 'infinity'
		float x_min = 1e20, y_min = 1e20, x_max = -1e20, y_max = -1e20;
		// Project all the intersections to the base plane
		for (int i = 0; i < intersections.size(); ++i) {
			intersections[i] -= plane_normal * distance(intersections[i], m_base_plane);
		}
		// Transform all the intersections form world-space to projector's projection-space
		for (int i = 0; i < intersections.size(); ++i) {
			intersections[i] = transformPoint(projector_view_proj_mat, intersections[i]);
			const glm::vec3 &p = intersections[i];
			x_min = std::min(x_min, p.x);
			y_min = std::min(y_min, p.y);
			x_max = std::max(x_max, p.x);
			y_max = std::max(y_max, p.y);
		}
		//printf("x:[%f %f], y:[%f %f]\n", x_min, x_max, y_min, y_max);

		glm::mat4 projector_view_proj_mat_inv = glm::inverse(projector_view_proj_mat);

		glm::mat4 pack(x_max - x_min,				0,	0,	x_min,
									0,	y_max - y_min,	0,	y_min,
									0,				0,	1,		0,
									0,				0,	0,		1);
		pack = glm::transpose(pack);
		m_range_matrix = projector_view_proj_mat_inv * pack;

		return true;
	}
}

glm::vec3 ProjectedGrid::getCorner(float u, float v) {
	glm::vec3 p_z0 = transformPoint(m_range_matrix, glm::vec3(u, v, -1.f));
	glm::vec3 p_z1 = transformPoint(m_range_matrix, glm::vec3(u, v, +1.f));
	return intersection(Line(p_z0, p_z1), m_base_plane);
}

glm::vec4 ProjectedGrid::getCorner4(float u, float v) {
	// this is hacky.. this does take care of the homogenous coordinates in a correct way, 
	// but only when the plane lies at y=0
	glm::vec4 origin(u, v, -1.f, 1.f);
	glm::vec4 direction(u, v, 1.f, 1.f);

	origin = transformPoint(m_range_matrix, origin);
	direction = transformPoint(m_range_matrix, direction);

	direction -= origin;
	float l = -origin.y / direction.y;	// assumes the plane is y=0
	glm::vec4 worldPos = origin + direction * l;
	return worldPos;
}

#define INTERPOLATE_VERSION_1

void ProjectedGrid::renderGeometry() {
	const int sides = m_options.sides;
	int index = 0;

#ifdef INTERPOLATE_VERSION_1
	glm::vec4 t_corners0 = getCorner4(0.f, 0.f);
	glm::vec4 t_corners1 = getCorner4(1.f, 0.f);
	glm::vec4 t_corners2 = getCorner4(0.f, 1.f);
	glm::vec4 t_corners3 = getCorner4(1.f, 1.f);

	float u = 0.f, v = 0.f;
	float du = 1.f / (float)(sides - 1);
	float dv = 1.f / (float)(sides - 1);
	//Method #1
	glm::vec4 result;
	for (int iv = 0; iv < sides; ++iv) {
		u = 0.f;
		for (int iu = 0; iu < sides; ++iu) {
			result.x = (1.0f-v)*( (1.0f-u)*t_corners0.x + u*t_corners1.x ) + v*( (1.0f-u)*t_corners2.x + u*t_corners3.x );				
			result.z = (1.0f-v)*( (1.0f-u)*t_corners0.z + u*t_corners1.z ) + v*( (1.0f-u)*t_corners2.z + u*t_corners3.z );				
			result.w = (1.0f-v)*( (1.0f-u)*t_corners0.w + u*t_corners1.w ) + v*( (1.0f-u)*t_corners2.w + u*t_corners3.w );				
		
			float divide = 1.0f/result.w;				
			result.x *= divide;
			result.z *= divide;

			m_vertices[index].x = result.x;
			m_vertices[index].z = result.z;
			m_vertices[index].y = 0.f;	// @hack: need to read the heightmap here
			
			++index;
			u += du;
		}
		v += dv;
	}
#else
	// #2: Slower version
	glm::vec3 t_corners0 = getCorner(0.f, 0.f);
	glm::vec3 t_corners1 = getCorner(1.f, 0.f);
	glm::vec3 t_corners2 = getCorner(0.f, 1.f);
	glm::vec3 t_corners3 = getCorner(1.f, 1.f);
	for (int iv = 0; iv < sides; ++iv) {
		for (int iu = 0; iu < sides; ++iu) {
			float u = (float)iu / (float)(sides - 1);
			float v = (float)iv / (float)(sides - 1);
			glm::vec3 p = getCorner(u, v);
			m_vertices[index].x = p.x;
			m_vertices[index].z = p.z;
			m_vertices[index].y = 0.f;	// @hack
			++index;
		}
	}
#endif

	glPushAttrib(GL_CURRENT_COLOR);
	glColor3f(0.f, 1.f, 0.f);

	// Draw the grid
	glBegin(GL_POINTS);
	for (int i = 0; i < m_vertices.size(); ++i) {
		glVertex3fv(glm::value_ptr(m_vertices[i]));
	}
	glEnd();
	
	glPopAttrib();
}