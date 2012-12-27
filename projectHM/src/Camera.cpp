#include "projectHM_PCH.h"

#include "Camera.h"

void Camera::updateViewMatrix() {
	// The target direction
	m_direction_tar = glm::vec3(
		cos(m_rotation.y) * sin(m_rotation.x),
		sin(m_rotation.y),
		cos(m_rotation.y) * cos(m_rotation.x)
		);
	// Right vector
	m_direction_right = glm::vec3(
		sin(m_rotation.x - PI * 0.5f),
		0,
		cos(m_rotation.x - PI * 0.5f)
		);
	m_direction_upv = glm::cross(m_direction_right, m_direction_tar);
	m_worldToCamera = glm::lookAt(m_position, m_position + m_direction_tar, m_direction_upv);
	m_cameraToWorld = glm::inverse(m_worldToCamera);
}

void Camera::updateProjectionMatrix() {
	m_cameraToScreen = glm::perspective(m_fov, (float)m_width / (float)m_height, m_zNear, m_zFar);
	m_screenToCamera = glm::inverse(m_cameraToScreen);
}

const glm::mat4& Camera::getProjectionMatrix() {
	return m_cameraToScreen;
}

const glm::mat4& Camera::getViewMatrix() {
	return m_worldToCamera;
}

const glm::mat4 Camera::getViewProjectionMatrix() const {
	return m_cameraToScreen * m_worldToCamera;
}

float* Camera::getPositionPtr() {
	return glm::value_ptr(m_position);
}

float* Camera::getViewMatrixInvPtr() {
	return glm::value_ptr(m_cameraToWorld);
}

void Camera::saveParasToFile(const char *filename) const {
	FILE *writter = fopen(filename, "w");
	if (writter == NULL) {
		fprintf(stderr, "Cannot save camera's settings into file '%s'\n", filename);
		return;
	}
	fprintf(writter, "%f %f %f\n", m_position.x, m_position.y, m_position.z);
	fprintf(writter, "%f %f\n", m_rotation.x, m_rotation.y);
	fclose(writter);
	printf("Successfully saved camera's settings into file '%s'\n", filename);
}

void Camera::loadParasFromFile(const char *filename) {
	FILE *reader = fopen(filename, "r");
	if (reader == NULL) {
		fprintf(stderr, "Cannot read camera's settings from file '%s'\n", filename);
		return;
	}
	fscanf(reader, "%f %f %f", &m_position.x, &m_position.y, &m_position.z);
	fscanf(reader, "%f %f\n", &m_rotation.x, &m_rotation.y);
	fclose(reader);
	updateViewMatrix();
	printf("Successfully loaded camera's settings from file '%s'\n", filename);
}
