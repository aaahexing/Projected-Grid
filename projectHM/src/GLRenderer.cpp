#include "projectHM_PCH.h"

#include "gl/glew.h"
#include "gl/glut.h"

#include "GLRenderer.h"

#include "../../hxlib/include/CGEffectManager.h"

#include "Scene.h"
#include "Camera.h"

void GLRenderer::init() {
	m_scene = NULL;
	m_camera = NULL;
	m_framebuffer = NULL;
	// Rendering parameters
	m_wireframe = false;
	// Load effects for OpenGL preview
	m_effect_manager = new CGEffectManager();
	m_effect_manager->registerStates();
	m_effect_manager->loadEffectFromFile("../data/shaders/material_effect.cgfx");
	m_effect_manager->activeFirstTechnique();
}

void GLRenderer::render(void (__cdecl *render_callback)()) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, m_width, m_height);
	glMatrixMode(GL_PROJECTION);
	glm::mat4 projection_mat = m_camera->getProjectionMatrix();
	glLoadMatrixf(glm::value_ptr(projection_mat));
	glm::mat4 model_mat(1.f);
	glm::mat4 view_mat = m_camera->getViewMatrix();
	glm::mat4 model_view = view_mat * model_mat;
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(glm::value_ptr(model_view));

	glPolygonMode(GL_FRONT_AND_BACK, m_wireframe ? GL_LINE : GL_FILL);

	// Render scene
	for (m_effect_manager->techniqueInitFirstPass(); m_effect_manager->techniqueRunning(); m_effect_manager->techniqueNextPass()) {
		m_effect_manager->techniqueApply();
		if (render_callback)
			(*render_callback)();
		else
			m_scene->drawScene();
	}

	glFinish();
	glutSwapBuffers();
}

void GLRenderer::shutdown() {
	delete m_effect_manager;
}

void GLRenderer::switchWireframe() {
	m_wireframe = !m_wireframe;
}

void GLRenderer::useNextTechnique() {
	m_effect_manager->activeNextTechnique();
}
