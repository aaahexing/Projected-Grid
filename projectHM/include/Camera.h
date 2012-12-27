#ifndef __CAMERA_H__
#define __CAMERA_H__

class Camera {
public:
	Camera(const Camera &c) {
		*this = c;
		updateViewMatrix();
		updateProjectionMatrix();
	}
	Camera(const glm::vec3 &_position = glm::vec3(0.f), float h_angle = PI, float v_angle = 0.f)
		: m_position(_position), m_zNear(0.001f), m_zFar(100.f) {
		setRotation(v_angle, h_angle);
		updateViewMatrix();
		updateProjectionMatrix();
	}

	// Modifiations on the projection matrix
	inline void setFOV(float fov) {
		m_fov = fov;
		updateProjectionMatrix();
	}
	inline void setFarClip(float zFar) {
		m_zFar = zFar;
		updateProjectionMatrix();
	}
	inline void setNearClip(float zNear) {
		m_zNear = zNear;
		updateProjectionMatrix();
	}
	inline void setScreenWindow(int _width, int _height) {
		m_width = _width;
		m_height = _height;
		updateProjectionMatrix();
	}
	inline float getFOV() const {
		return m_fov;
	}
	inline float getFarClip() const {
		return m_zFar;
	}
	inline float getNearClip() const {
		return m_zNear;
	}
	inline float getWidth() const {
		return m_width;
	}
	inline float getHeight() const {
		return m_height;
	}
	inline float getClipHeight() const {
		return m_zNear * tan(m_fov / 180.0f * PI * 0.5f) * 2;
	}
	inline glm::vec3 getPosition() const {
		return m_position;
	}
	inline glm::vec3 getDirection() const {
		return m_direction_tar;
	}

	// Modifications on the view matrix
	// Move the camera
	inline void moveX(float dis) {
		m_position += m_direction_right * dis;
		updateViewMatrix();
	}
	inline void moveY(float dis) {
		m_position += m_direction_upv * dis;
		updateViewMatrix();
	}
	inline void moveZ(float dis) {
		m_position += m_direction_tar * dis;
		updateViewMatrix();
	}
	inline void addRotation(const glm::vec2 &_rotation) {
		m_rotation += _rotation;
		updateViewMatrix();
	}
	inline void addRotation(float h_angle, float v_angle) {
		m_rotation.x += h_angle;
		m_rotation.y += v_angle;
		updateViewMatrix();
	}
	inline void setPosition(const glm::vec3 &_position) {
		m_position = _position;
		updateViewMatrix();
	}
	inline void setPotision(float px, float py, float pz) {
		m_position = glm::vec3(px, py, pz);
		updateViewMatrix();
	}
	inline void setRotation(const glm::vec2 &_rotation) {
		m_rotation = _rotation;
		updateViewMatrix();
	}
	inline void setRotation(float h_angle, float v_angle) {
		m_rotation = glm::vec2(h_angle, v_angle);
		updateViewMatrix();
	}
	inline void setDirection(const glm::vec3 &_direction) {
		glm::vec3 dir_norm = glm::normalize(_direction);
		m_rotation.x = atan2(dir_norm.z, dir_norm.x);
		m_rotation.y = asin(dir_norm.y);
		updateViewMatrix();
	}

	void updateViewMatrix();
	void updateProjectionMatrix();
	const glm::mat4& getProjectionMatrix();
	const glm::mat4& getViewMatrix();
	const glm::mat4 getViewProjectionMatrix() const;
	void saveParasToFile(const char *filename) const;
	void loadParasFromFile(const char *filename);
	// For GPU ray propagation
	float* getPositionPtr();
	float* getViewMatrixInvPtr();

protected:
	glm::vec2 m_rotation;						// camera's rotation
	glm::vec3 m_position;						// camera's position
	glm::vec3 m_direction_tar;
	glm::vec3 m_direction_upv;
	glm::vec3 m_direction_right;
	int m_width, m_height;
	float m_fov, m_zNear, m_zFar;
	// Matrices
	glm::mat4 m_worldToCamera, m_cameraToWorld;		// view matrix (world space -> camera(eye) space)
	glm::mat4 m_cameraToScreen, m_screenToCamera;	// projection matrix (camera(eye) space -> clip space)
	//glm::mat4 m_cameraToRaster, m_rasterToCamera;	// (clip space -> NDC space, a [-1, 1]^3 cube -> Image space)
};

#endif	/* __CAMERA_H__ */
