#include "projectHM_PCH.h"

#include <cstdio>

#include <gl/glew.h>
#include <gl/glut.h>

#include "../../hxlib/include/WindowsTimer.h"
#include "../../hxlib/include/OpenGLWrapper.h"

#include "Scene.h"
#include "Shape.h"
#include "Camera.h"
#include "Transform.h"
#include "GLRenderer.h"
#include "ProjectedGrid.h"

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

ProjectedGrid proj_grid(
	Plane(glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 1.f, 0.f)),
	&camera,
	ProjectedGridOptions()
	);

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

	if (proj_grid.getRangeMatrix(1.f, -1.f, 3.f)) {
		proj_grid.renderGeometry();
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
