#include "projectHM_PCH.h"

#include "gl/glew.h"
#include "gl/glut.h"

#include "Scene.h"

void Scene::drawScene() const {
	glutSolidSphere(1.0, 100, 100);
}
