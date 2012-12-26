#include <cstdio>

#include "OpenGLWrapper.h"

GLuint GLBufferAgent::createBuffer(GLenum target, GLuint size, const GLvoid* data, GLenum usage) {
	GLuint buffer = 0;
	glGenBuffers(1, &buffer);
	if (buffer == 0) {
		fprintf(stderr, "Cannot create element buffer with size = %u bytes\n", size);
		return 0;
	}
	glBindBuffer(target, buffer);
	glBufferData(target, size, data, usage);
	glBindBuffer(target, 0);
	return buffer;
}
