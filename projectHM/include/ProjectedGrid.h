#ifndef __PROJECTEDGRID_H__
#define __PROJECTEDGRID_H__

#include "Shape.h"

class Camera;

/*
	The steps of the algorithm:
	1. Get the position and direction of the camera and all other parameters 
		required to recreate the camera matrices.
	2. Determine if any part of the displaceable volume is within the camera
		frustum. Abort rendering of the surface otherwise. AKA, build the range
		matrix:
		a. Transform the corner-points (¡À1, ¡À1, ¡À1) of the camera frustum into world-space by using the
			camera¡¯s inverted viewproj matrix.
		b. Check for intersections between the edges of the camera frustum and the bound planes (S_upper and
			S_lower). Store the world-space positions of all intersections in a buffer.
		c. If any of the frustum corner-points lies between the bound-planes, add them to the buffer as well.
		d. If there are no points in the buffer, then V_cam and V_displaceable do not intersect and the surface does
			not have to be rendered.
		e. Project all the points in the buffer onto S_base.
		f. Transform the points in the buffer to projector-space by using the inverse of the M_projector matrix.
		The x- and y-span of Vvisible is now defined as the minimum/maximum x/y-values of the points in
		the buffer.
		g. Build a matrix (M_range) that transform the [0..1] range of the x and y-coordinates into the [x_min..x_max]
		and [y_min..y_max] ranges but leave the z and w values intact.
			[ x_max - x_min			0			0			x_min	]
			[		0			y_max - y_min	0			y_min	]
			[		0				0			1			0		]
			[		0				0			0			1		]
	3. Aim the projector as described in Section 2.4.1. This will provide a new position and direction for the
		projector (that could be slightly different from the camera). Use the standard ¡°look at¡±-method to
		create a view-matrix (M_pview) for the projector based on the new position and forward vector. This will
		allow M_projector to be calculated. MPerspective is inherited from the rendering camera.
	4. Consider the resulting volume (V_visible) of the intersection between V_cam and V_displaceable. Calculate the x and
		y-span of V_visible in projector space. Construct a range conversion matrix (M_range) that transforms the [0,
		1] range to those spans. Update M_projector with the range conversion matrix:
	5. Create a grid with x= [0..1], y = [0..1]. A pair of texture coordinates (u, v) should be set to u = x, v = y
		for later use.
	6. For each vertex in the grid, transform it two times with M_projector, first with the z-coordinates set to -1
		and then with the z-coordinate set to 1. The final vertex is the intersection of the line between these
		two vertices and Sbase. This intersection only has to be done for the corners of the grid, the rest of the
		vertices can be obtained by using linear interpolation. As long as homogenous coordinates are used,
		the interpolated version should be equivalent. (see Appendix B for the line-plane intersection of
		homogenous coordinates)
	7. Displace the vertices along N_base by the amount defined by the height field (fHF) to get the final vertex
		in world space.
*/

struct ProjectedGridOptions {
	int sides;
	bool smooth;
	float strength;
	float elevation;
public:
	ProjectedGridOptions()
		: sides(256), strength(35.0), elevation(50.0), smooth(false) {}
};

class ProjectedGrid {
	ProjectedGridOptions m_options;

	glm::mat4 m_range_matrix;
	Camera *m_projecting_camera;
	const Camera *m_rendering_camera;
	Plane m_base_plane, m_upper_bound_plane, m_lower_bound_plane;

	std::vector<glm::vec3> m_vertices;
	glm::vec4 t_corners0, t_corners1, t_corners2, t_corners3;
public:
	ProjectedGrid(const Plane &base_plane, const Camera *camera, const ProjectedGridOptions &options);
	~ProjectedGrid();

	void setOptions(const ProjectedGridOptions &options);

	bool getRangeMatrix(float water_max_height, float water_min_height, float projector_height_inc);

	glm::vec3 getCorner(float u, float v);

	glm::vec4 getCorner4(float u, float v);

	void renderGeometry();

	// for debugging
};

#endif	/* __PROJECTEDGRID_H__ */