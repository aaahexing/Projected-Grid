#include "projectHM_PCH.h"

#include <cstdio>

#include <gl/glew.h>
#include <gl/glut.h>

#include "Scene.h"
#include "Shape.h"
#include "Camera.h"
#include "Transform.h"
#include "GLRenderer.h"

#include "../../hxlib/include/WindowsTimer.h"
#include "../../hxlib/include/OpenGLWrapper.h"

const int screenWidth = 1280;
const int screenHeight = 720;

Scene scene;
GLRenderer gl_renderer;
Camera camera(glm::vec3(0, 2, 6), -PI / 30, PI);

bool key_down[256];
int pre_x, pre_y, button_mask = 0;

WindowsTimer walk_timer;

const float walk_speed = 0.004f;
const float mouse_speed = 0.001f;

// Helper functions for debugging
inline void debugVec3(int i, const glm::vec3 &v) {
	printf("%d : %f %f %f\n", i, v.x, v.y, v.z);
}

inline void debugVec4(int i, const glm::vec4 &v) {
	printf("%d : %f %f %f %f\n", i, v.x, v.y, v.z, v.w);
}

inline void debugMat4(const float* m) {
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			printf("%f ", m[i * 4 + j]);
		}
		puts("");
	}
}

inline void debugMat4(const glm::mat4 &m) {
	debugMat4(glm::value_ptr(m));
}

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

// @hack: assume there's no displacement, so S_upper = S_lower = S_base
Plane plane(glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
Plane upper_bound(plane), lower_bound(plane);

glm::mat4 projector_mat;

Camera *proj_camera = NULL;

struct GlobalOption {
	float strength;
	float elevation;
	bool smooth;
} options;

// Get the range matrix
bool getProjectorMatrix() {
	int n_points = 0;
	glm::vec3 frustum[8];	// corners of camera's frustum
	glm::vec3 projected_points[48];
	const int cube_edges[] = {
		0, 1,
		0, 2,
		2, 3,
		1, 3,
		0, 4,
		2, 6,
		3, 7,
		1, 5,
		4, 6,
		4, 5,
		5, 7,
		6, 7
	};
	// transform frustum points to world-space
	glm::mat4 view_proj_mat = camera.getViewProjectionMatrix();
	glm::mat4 view_proj_mat_inv = glm::inverse(view_proj_mat);
	frustum[0] = transformPoint(view_proj_mat_inv, glm::vec3(-1.f, -1.f, 0.f));
	frustum[1] = transformPoint(view_proj_mat_inv, glm::vec3(+1.f, -1.f, 0.f));
	frustum[2] = transformPoint(view_proj_mat_inv, glm::vec3(-1.f, +1.f, 0.f));
	frustum[3] = transformPoint(view_proj_mat_inv, glm::vec3(+1.f, +1.f, 0.f));
	frustum[4] = transformPoint(view_proj_mat_inv, glm::vec3(-1.f, -1.f, 1.f));
	frustum[5] = transformPoint(view_proj_mat_inv, glm::vec3(+1.f, -1.f, 1.f));
	frustum[6] = transformPoint(view_proj_mat_inv, glm::vec3(-1.f, +1.f, 1.f));
	frustum[7] = transformPoint(view_proj_mat_inv, glm::vec3(+1.f, +1.f, 1.f));

	for (int i = 0; i < 8; ++i) {
		const glm::vec3 &p = frustum[i];
		if ((upper_bound.a*p.x + upper_bound.b*p.y + upper_bound.c*p.z + upper_bound.d)
			/(lower_bound.a*p.x + lower_bound.b*p.y + lower_bound.c*p.z + lower_bound.d) < 0.f) {
			projected_points[n_points++] = frustum[i];
		}
	}
	// get the intersections with upper_bound and lower_bound
	for (int ei = 0; ei < 12; ++ei) {
		int src = cube_edges[ei * 2];
		int tar = cube_edges[ei * 2 + 1];
		const glm::vec3 src_v = frustum[src];
		const glm::vec3 tar_v = frustum[tar];
		Line line(frustum[src], frustum[tar]);
		if ((upper_bound.a*src_v.x + upper_bound.b*src_v.y + upper_bound.c*src_v.z + upper_bound.d)
			/(upper_bound.a*tar_v.x + upper_bound.b*tar_v.y + upper_bound.c*tar_v.z + upper_bound.d) < 0.f) {
			projected_points[n_points++] = intersection(line, upper_bound);
		}
		if ((lower_bound.a*src_v.x + lower_bound.b*src_v.y + lower_bound.c*src_v.z + lower_bound.d)
			/(lower_bound.a*tar_v.x + lower_bound.b*tar_v.y + lower_bound.c*tar_v.z + lower_bound.d) < 0.f) {
			projected_points[n_points++] = intersection(line, lower_bound);
		}
	}

	if (n_points == 0) {
		return false;
	}

	if (proj_camera) {
		delete proj_camera;
		proj_camera = NULL;
	}
	// @todo: try to optimize the projecting camera
	proj_camera = new Camera(camera);
	glm::vec3 proj_position = proj_camera->getPosition();
	float height_in_plane = lower_bound.a * proj_position.x
		+ lower_bound.b * proj_position.y
		+ lower_bound.c * proj_position.z;

	bool keep_it_simple = false;
	bool under_water = false;

	if (height_in_plane < 0.f) {
		under_water = true;
	}

	if (keep_it_simple) {
		// do nothing
		proj_camera->setDirection(camera.getDirection());
	} else {
		glm::vec3 aimpoint0, aimpoint1;
		if (height_in_plane < options.strength + options.elevation) {
			if (under_water) {
				proj_camera->setPosition(proj_camera->getPosition() 
					+ lower_bound.getNormal() * (options.strength + options.elevation - 2 * height_in_plane));
			} else {
				proj_camera->setPosition(proj_camera->getPosition()
					+ lower_bound.getNormal() * (options.strength + options.elevation - height_in_plane));
			}
		}

		// Aim the projector at the point where the camera view-vector intersects the plane
		// if the camera is aimed away from the plane, mirror it's view-vector against the plane
		if (glm::dot(plane.getNormal(), camera.getDirection()) < 0.f
			|| glm::dot(plane.getNormal(), camera.getPosition()) < 0.f) {
			aimpoint0 = intersection(Line(camera.getPosition(), camera.getPosition() + camera.getDirection()), plane);
		} else {
			glm::vec3 flipped = camera.getDirection() - plane.getNormal() * 2.f * glm::dot(camera.getDirection(), plane.getNormal());
			aimpoint0 = intersection(Line(camera.getPosition(), camera.getPosition() + flipped), plane);
		}

		// force the point the camera is looking at in a plane, and have the projector look at it
		// works well against horizon, even when camera is looking upwards
		// doesn't work straight down/up
		float af = fabs(glm::dot(plane.getNormal(), camera.getDirection()));
		//af = 1 - (1-af)*(1-af)*(1-af)*(1-af)*(1-af);
		//aimpoint2 = (rendering_camera->position + rendering_camera->zfar * rendering_camera->forward);
		aimpoint1 = (camera.getPosition() + 10.0f * camera.getDirection());
		aimpoint1 = aimpoint1 - plane.getNormal() * glm::dot(aimpoint1, plane.getNormal());

		// fade between aimpoint & aimpoint2 depending on view angle

		aimpoint0 = aimpoint0 * af + aimpoint1 * (1.0f - af);
		//aimpoint = aimpoint2;

		proj_camera->setDirection(aimpoint0 - proj_camera->getPosition());
	}

	glm::vec3 normal = plane.getNormal();
	glm::mat4 view_mat = proj_camera->getViewMatrix();
	glm::mat4 projection_mat = proj_camera->getProjectionMatrix();
	for (int i = 0; i < n_points; ++i) {
		// Project the intersections to S_base
		projected_points[i] = projected_points[i] - normal * glm::dot(projected_points[i], normal);
		// Transform them from world-space to the projection-space
		projected_points[i] = transformPoint(view_mat, projected_points[i]);
		projected_points[i] = transformPoint(projection_mat, projected_points[i]);
	}
	// @todo: need definition of 'infinity'
	float x_min = 1e20, y_min = 1e20, x_max = -1e20, y_max = -1e20;
	for (int i = 0; i < n_points; ++i) {
		const glm::vec3 &p = projected_points[i];
		x_min = std::min(x_min, p.x);
		y_min = std::min(y_min, p.y);
		x_max = std::max(x_max, p.x);
		y_max = std::max(y_max, p.y);
	}
	printf("x:[%f %f], y:[%f %f]\n", x_min, x_max, y_min, y_max);
	glm::mat4 range_mat(x_max - x_min,				0,	0,	x_min,
									0,	y_max - y_min,	0,	y_min,
									0,				0,	1,		0,
									0,				0,	0,		1);
	// @todo: need a transpose?
	range_mat = glm::transpose(range_mat);
	projector_mat = range_mat * glm::inverse(proj_camera->getViewProjectionMatrix());
	return true;
}

glm::vec4 calWorldPosition(float u, float v, const glm::mat4 &m) {
	// this is hacky.. this does take care of the homogenous coordinates in a correct way, 
	// but only when the plane lies at y=0
	glm::vec4 origin(u, v, -1.f, 1.f);
	glm::vec4 direction(u, v, 1.f, 1.f);

	origin = transformPoint(m, origin);
	direction = transformPoint(m, direction);

#ifndef O_version
	direction -= origin;
	float l = -origin.y / direction.y;	// assumes the plane is y=0
	glm::vec4 worldPos = origin + direction * l;
	return worldPos;
#else
	//glm::vec3 org(origin.x / origin.w, origin.y / origin.w, origin.z / origin.w);
	//glm::vec3 dir(direction.x / direction.w, direction.y / direction.w, direction.z / direction.w);
	//dir -= org;
	//dir = glm::normalize(dir);

	//glm::vec3 worldPos = intersection(Line(org, dir), plane);

	//glm::vec4 tmp = camera.getViewMatrix() * glm::vec4(worldPos.x, worldPos.y, worldPos.z, 1.f);
	//float temp = -tmp.z / tmp.w;
	//glm::vec4 retPos(worldPos.x, worldPos.y, worldPos.z, 1.f);
	//retPos /= temp;
	//return retPos;
#endif
}

const int sizeY = 20;
const int sizeX = 20;

glm::vec3 vertices[12800];

bool renderGeometry(const glm::mat4 &m) {
	glm::vec4 t_corners0 = calWorldPosition(0.f, 0.f, m);
	glm::vec4 t_corners1 = calWorldPosition(1.f, 0.f, m);
	glm::vec4 t_corners2 = calWorldPosition(0.f, 1.f, m);
	glm::vec4 t_corners3 = calWorldPosition(1.f, 1.f, m);

	float du = 1.f / (float)(sizeX - 1);
	float dv = 1.f / (float)(sizeY - 1);
	float u = 0.f, v = 0.f;
	int i = 0;
	glm::vec4 result;
	for (int iv = 0; iv < sizeY; ++iv) {
		u = 0.f;
		for (int iu = 0; iu < sizeX; ++iu) {
			//result = (1.0f-v)*( (1.0f-u)*t_corners0 + u*t_corners1 ) + v*( (1.0f-u)*t_corners2 + u*t_corners3 );				
			result.x = (1.0f-v)*( (1.0f-u)*t_corners0.x + u*t_corners1.x ) + v*( (1.0f-u)*t_corners2.x + u*t_corners3.x );				
			result.z = (1.0f-v)*( (1.0f-u)*t_corners0.z + u*t_corners1.z ) + v*( (1.0f-u)*t_corners2.z + u*t_corners3.z );				
			result.w = (1.0f-v)*( (1.0f-u)*t_corners0.w + u*t_corners1.w ) + v*( (1.0f-u)*t_corners2.w + u*t_corners3.w );				

			float divide = 1.0f/result.w;				
			result.x *= divide;
			result.z *= divide;

			vertices[i].x = result.x;
			vertices[i].z = result.z;
			//vertices[i].y = get_height_at(magnitude*result.x, magnitude*result.z, octaves);
			//vertices[i].y = get_height_dual(magnitude*result.x, magnitude*result.z );
			vertices[i].y = 0.f;	// @hack

			//printf("%f , %f\n", vertices[i].x, vertices[i].y);

			i++;
			u += du;
		}
		v += dv;
	}

	//// smooth the heightdata
	//if(options.smooth)
	//{
	//	//for(int n=0; n<3; n++)
	//	for(int v=1; v<(sizeY-1); v++)
	//	{
	//		for(int u=1; u<(sizeX-1); u++)
	//		{				
	//			vertices[v*sizeX + u].y =	0.2f * (vertices[v*sizeX + u].y +
	//				vertices[v*sizeX + (u+1)].y + 
	//				vertices[v*sizeX + (u-1)].y + 
	//				vertices[(v+1)*sizeX + u].y + 
	//				vertices[(v-1)*sizeX + u].y);															
	//		}
	//	}
	//}

	return true;
}

void renderProjectedGrids() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, screenWidth, screenWidth);
	glMatrixMode(GL_PROJECTION);
	glm::mat4 projection_mat = camera.getProjectionMatrix();
	glLoadMatrixf(glm::value_ptr(projection_mat));
	glm::mat4 model_mat(1.f);
	glm::mat4 view_mat = camera.getViewMatrix();
	glm::mat4 model_view = view_mat * model_mat;
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(glm::value_ptr(model_view));

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	options.strength = 0.f;
	options.elevation = 0.f;
	options.smooth = false;
	if (getProjectorMatrix()) {
		puts("ssssssssssssssssssss");
		debugMat4(projector_mat);
		renderGeometry(projector_mat);

		glBegin(GL_POINTS);
		for (int i = 0; i < sizeX * sizeY; ++i) {
			const glm::vec3 p = vertices[i];
			glVertex3f(p.x, p.y, p.z);
		}
		glEnd();
	}


	glFinish();
	glutSwapBuffers();
}

void displayCallback() {
	renderProjectedGrids();
	/*
	gl_renderer.render(NULL);
	*/
}

void idleCallback() {
	float distance = walk_speed * walk_timer.getMicroseconds();
	if (key_down['s']) {
		camera.moveZ(-distance);
	}
	if (key_down['w']) {
		camera.moveZ(distance);
	}
	if (key_down['a']) {
		camera.moveX(-distance);
	}
	if (key_down['d']) {
		camera.moveX(distance);
	}
	if (key_down['e']) {
		camera.moveY(-distance);
	}
	if (key_down['q']) {
		camera.moveY(distance);
	}

	glutPostRedisplay();
}

void motionCallback(int x, int y) {
	float dx = x - pre_x;
	float dy = y - pre_y;
	if (button_mask & (1 << GLUT_LEFT_BUTTON)) {
		camera.addRotation(-mouse_speed * dx, -mouse_speed * dy);
	}
	pre_x = x, pre_y = y;
	glutPostRedisplay();
}

void mouseCallback(int button, int state, int x, int y) {
	if (state == GLUT_DOWN) {
		button_mask |= (1 << button);
	} else if (state == GLUT_UP) {
		button_mask -= (1 << button);
	}
	pre_x = x, pre_y = y;
	glutPostRedisplay();
}

void destroyWorld() {
	// Remember to call it to avoid memory leaks
	gl_renderer.shutdown();
}

void keyboardCallback(unsigned char key, int /*x*/, int /*y*/) {
	switch (key) {
	case 27:
		destroyWorld();
		exit(0);
		break;
	case '1':
		gl_renderer.useNextTechnique();
		break;
	case '2':
		gl_renderer.switchWireframe();
		break;
	case 'S':
		camera.saveParasToFile("../data/scenes/camera.cfg");
		break;
	case 'L':
		camera.loadParasFromFile("../data/scenes/camera.cfg");
		break;
	default:
		break;
	}
	key_down[key] = true;

	glutPostRedisplay();
}

void keyboardUpCallback(unsigned char key, int /*x*/, int /*y*/) {
	key_down[key] = false;
}

void goIntoWorld(const char *scene_file_name, int argc, char *argv[]) {
	// Init GLUT Context
	glutInit(&argc, argv);
	glutInitWindowPosition(200, 200);
	glutInitWindowSize(screenWidth, screenHeight);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutCreateWindow("Project Height Map Demo");
	// Init GLEW
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to init GLEW!\n");
		exit(-1);
	}
	// Other variables after GL Context created
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	// Init callbacks
	//glutReshapeFunc(reshapeCallback);
	glutDisplayFunc(displayCallback);
	glutMotionFunc(motionCallback);
	glutMouseFunc(mouseCallback);
	glutPassiveMotionFunc(motionCallback);
	glutKeyboardFunc(keyboardCallback);
	glutKeyboardUpFunc(keyboardUpCallback);
	glutIdleFunc(idleCallback);

	// Init the renderer
	gl_renderer.init();
	gl_renderer.setScene(&scene);
	gl_renderer.setCamera(&camera);
	gl_renderer.setWindow(screenWidth, screenHeight);
	// Init the camera
	camera.setFOV(45.f);
	camera.setFarClip(100.f);
	camera.setNearClip(0.01f);
	camera.setScreenWindow(screenWidth, screenHeight);

	glutMainLoop();
}

int main(int argc, char *argv[]) {
	goIntoWorld("no_use_path", argc, argv);
	return 0;
}
