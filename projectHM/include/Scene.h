#ifndef __SCENE_H__
#define __SCENE_H__

class Scene {
public:
	inline const glm::mat4& getModelMatrix() const {
		return modelToWorld;
	}

	// Rendering methods for OpenGL interoperation
	void drawScene() const;

public:
	glm::mat4 modelToWorld;
};

#endif	/* __SCENE_H__ */
