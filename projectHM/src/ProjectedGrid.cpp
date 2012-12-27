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
		return false;
	} else {
		float base_dist = 0.f;
		float upper_dist = base_dist + water_max_height;
		float lower_dist = base_dist + water_min_height;
		const glm::vec3 plane_normal = m_base_plane.getNormal();
		// Project all the intersections to the base plane
		for (int i = 0; i < intersections.size(); ++i) {
			intersections[i] -= plane_normal * distance(intersections[i], m_base_plane);
		}
		// Locate the projector for projected grid
		glm::vec3 projector_pos;
		const glm::vec3 cam_pos = m_rendering_camera->getPosition();
		float dist_along_water_normal = distance(m_rendering_camera->getPosition(), m_base_plane);
		if (dist_along_water_normal >= upper_dist) {
			projector_pos = cam_pos + plane_normal * projector_height_inc;
		} else {
			if (dist_along_water_normal <= lower_dist) {
				float reflected_dist = base_dist - distance(cam_pos, m_base_plane);
				projector_pos = cam_pos + (projector_height_inc + base_dist + reflected_dist) * plane_normal;
			} else {
				float dist_above_cam = projector_height_inc + upper_dist - distance(cam_pos, m_base_plane);
				projector_pos = cam_pos + dist_above_cam * plane_normal;
			}
		}

		// Assert Projector's position is above the water's max height
		assert(distance(projector_pos, m_base_plane) > upper_dist);
		
		// Assert Projector's position is above the camera's position
		assert(glm::dot(projector_pos - cam_pos, plane_normal) > 0.f);

		//////////////////////////////////////////////////////////////////
		// Orient the Projector - That is, have the projector face towards 
		// a location between 2 end-points. The 1st end-point is computed 
		// by choosing a point a set number of distance infront of the 
		// camera and then projecting it down onto the water base plane. 
		// The 2nd end-point is the intersection of the camera's facing 
		// vector and the water base plane.
		//////////////////////////////////////////////////////////////////
		glm::vec3 projector_tar, aimpoint0, aimpoint1;
		glm::vec3 cam_dir = m_rendering_camera->getDirection();
		// Aim the projector at the point where the camera view-vector intersects the plane
		// if the camera is aimed away from the plane, mirror it's view-vector against the plane
		if (glm::dot(plane_normal, cam_dir) < 0.f || glm::dot(plane_normal, cam_pos) < 0.f) {
				aimpoint0 = intersection(Line(cam_pos, cam_pos + cam_dir), m_base_plane);
		} else {
			glm::vec3 flipped = cam_dir - plane_normal * 2.f * glm::dot(cam_dir, plane_normal);
			aimpoint0 = intersection(Line(cam_pos, cam_pos + flipped), m_base_plane);
		}

		// force the point the camera is looking at in a plane, and have the projector look at it
		// works well against horizon, even when camera is looking upwards
		// doesn't work straight down/up
		float af = fabs(glm::dot(plane_normal, cam_dir));
		//af = 1 - (1-af)*(1-af)*(1-af)*(1-af)*(1-af);
		//aimpoint2 = (rendering_camera->position + rendering_camera->zfar * rendering_camera->forward);
		aimpoint1 = (cam_pos + 10.0f * cam_dir);
		aimpoint1 = aimpoint1 - plane_normal * glm::dot(aimpoint1, plane_normal);
		// fade between aimpoint & aimpoint2 depending on view angle
		projector_tar = aimpoint0 * af + aimpoint1 * (1.0f - af);
		// Set the projector
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
		// Transform all the intersections form world-space to projector's projection-space
		for (int i = 0; i < intersections.size(); ++i) {
			intersections[i] = transformPoint(projector_view_proj_mat, intersections[i]);
			const glm::vec3 &p = intersections[i];
			x_min = std::min(x_min, p.x);
			y_min = std::min(y_min, p.y);
			x_max = std::max(x_max, p.x);
			y_max = std::max(y_max, p.y);
		}
		printf("x:[%f %f], y:[%f %f]\n", x_min, x_max, y_min, y_max);

		glm::mat4 projector_view_proj_mat_inv = glm::inverse(projector_view_proj_mat);

		glm::mat4 pack(x_max - x_min,				0,	0,	x_min,
									0,	y_max - y_min,	0,	y_min,
									0,				0,	1,		0,
									0,				0,	0,		1);
		m_range_matrix = projector_view_proj_mat_inv * pack;

		//
		//// Transform the 8 bounding coordinates to world coordinates, using the 
		//// min and max computed for the x and y axes in the Projector's 
		//// projective space. There's 8 bounding coordinates because of the 
		//// additional z axis.
		//
		//double z_min = -1.0, z_max = 1.0;
		////
		//glm::vec3 x0_y0_z0 = transformPoint(projector_view_proj_mat_inv, glm::vec3(x_min, y_min, z_min));
		//glm::vec3 x0_y0_z1 = transformPoint(projector_view_proj_mat_inv, glm::vec3(x_min, y_min, z_max));
		////
		//glm::vec3 x0_y1_z0 = transformPoint(projector_view_proj_mat_inv, glm::vec3(x_min, y_max, z_min));
		//glm::vec3 x0_y1_z1 = transformPoint(projector_view_proj_mat_inv, glm::vec3(x_min, y_max, z_max));
		////
		//glm::vec3 x1_y0_z0 = transformPoint(projector_view_proj_mat_inv, glm::vec3(x_max, y_min, z_min));
		//glm::vec3 x1_y0_z1 = transformPoint(projector_view_proj_mat_inv, glm::vec3(x_max, y_min, z_max));
		////
		//glm::vec3 x1_y1_z0 = transformPoint(projector_view_proj_mat_inv, glm::vec3(x_max, y_max, z_min));
		//glm::vec3 x1_y1_z1 = transformPoint(projector_view_proj_mat_inv, glm::vec3(x_max, y_max, z_max));

		//// Compute the 4 bounding coordinates on the water base plane for 
		//// each combination of min and max of x,y pairs. This is done by 
		//// computing the intersection of the (infinite) lines with the 
		//// water base plane.

		//glm::vec3 line_vec = x0_y0_z0 - x0_y0_z1;


		return true;
	}
}

glm::vec3 ProjectedGrid::getCorner(float u, float v) {
	glm::vec3 p_z0 = transformPoint(m_range_matrix, glm::vec3(u, v, -1.f));
	glm::vec3 p_z1 = transformPoint(m_range_matrix, glm::vec3(u, v, +1.f));
	return intersection(Line(p_z0, p_z1), m_base_plane);
}

bool ProjectedGrid::renderGeometry() {
	glm::vec4 t_corners0 = glm::vec4(getCorner(0.f, 0.f), 1.f);
	glm::vec4 t_corners1 = glm::vec4(getCorner(1.f, 0.f), 1.f);
	glm::vec4 t_corners2 = glm::vec4(getCorner(0.f, 1.f), 1.f);
	glm::vec4 t_corners3 = glm::vec4(getCorner(1.f, 1.f), 1.f);

	const int sides = m_options.sides;
	int index = 0;
	float u = 0.f, v = 0.f;
	float du = 1.f / (float)(sides - 1);
	float dv = 1.f / (float)(sides - 1);
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

	// Draw the grid
	glBegin(GL_POINTS);
	for (int i = 0; i < m_vertices.size(); ++i) {
		glVertex3fv(glm::value_ptr(m_vertices[i]));
	}
	glEnd();

	return true;
}