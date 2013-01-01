#ifndef __GLRENDERCONTROLER_H__
#define __GLRENDERCONTROLER_H__

class GLRenderControler {
public:
	GLRenderControler();

	inline void switchWireframe() {
		m_wireframe = !m_wireframe;
	}
	inline bool isWireframe() const {
		return m_wireframe;
	}

protected:
	bool m_wireframe;
};

#endif