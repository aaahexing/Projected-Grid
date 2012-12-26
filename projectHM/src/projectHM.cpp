#include "projectHM_PCH.h"

#include <cstdio>

#include <gl/glew.h>
#include <gl/glut.h>

#include "Scene.h"
#include "Camera.h"
#include "GLRenderer.h"

#include "../../hxlib/include/WindowsTimer.h"
#include "../../hxlib/include/OpenGLWrapper.h"

const int screenWidth = 1280;
const int screenHeight = 720;

Scene scene;
GLRenderer gl_renderer;
Camera camera(glm::vec3(0, 2, 6), 0, PI);

bool key_down[256];
int pre_x, pre_y, button_mask = 0;

WindowsTimer walk_timer;

const float walk_speed = 0.004f;
const float mouse_speed = 0.001f;

void displayCallback() {
	gl_renderer.render(NULL);			
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
	//goIntoWorld("../data/scenes/grids_convex_part.obj", argc, argv);
	//goIntoWorld("../data/scenes/grids_convex.obj", argc, argv);
	goIntoWorld("no_use_path", argc, argv);
	//goIntoWorld("../data/scenes/bigguy_part.obj", argc, argv);
	//goIntoWorld("../data/scenes/bigguy_part2.obj", argc, argv);
	return 0;
}
