#ifndef __RENDERER_H__
#define __RENDERER_H__

class Scene;
class Camera;

class Renderer {
public:
	inline void setScene(Scene *_scene) {
		m_scene = _scene;
	}
	inline void setCamera(Camera *_camera) {
		m_camera = _camera;
	}
	inline void setWindow(int _width, int _height) {
		m_width = _width, m_height = _height;
	}

	virtual void init() = 0;
	virtual void shutdown() = 0;
	virtual void render(void (__cdecl *render_callback)()) = 0;

protected:
	int m_width;
	int m_height;
	Scene *m_scene;
	Camera *m_camera;
	glm::vec3 *m_framebuffer;
};

#endif	/* __RENDERER_H__ */
