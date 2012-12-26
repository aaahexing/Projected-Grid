#ifndef __GLRENDERER_H__
#define __GLRENDERER_H__

#include "Renderer.h"

class Scene;
class CGEffectManager;

class GLRenderer : public Renderer {
public:
	virtual void init();
	virtual void shutdown();
	virtual void render(void (__cdecl *render_callback)());

	inline void setScene(Scene *_scene) {
		m_scene = _scene;
	}

	void switchWireframe();
	void useNextTechnique();

protected:
	Scene *m_scene;
	bool m_wireframe;
	CGEffectManager *m_effect_manager;
};

#endif	/* __GLRENDERER_H__ */
