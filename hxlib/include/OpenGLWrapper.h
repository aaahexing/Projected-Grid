#ifndef __GLBUFFERAGENT_H__
#define __GLBUFFERAGENT_H__

#include "gl/glew.h"

class GLBufferAgent {
public:
	static inline void bindPixelBufferPack(GLuint buffer) {
		glBindBuffer(GL_PIXEL_PACK_BUFFER, buffer);
	}
	static inline void unbindPixelBufferPack() {
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	}
	static inline void bindPixelBufferUnpack(GLuint buffer) {
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer);
	}
	static inline void unbindPixelBufferUnpack() {
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	}
	static inline void bindArrayBuffer(GLuint buffer) {
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
	}
	static inline void unbindArrayBuffer() {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	static inline void bindElementArrayBuffer(GLuint buffer) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
	}
	static inline void unbindElementArrayBuffer() {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	static inline GLuint createPixelBuffer(unsigned size, const void *data, GLenum usage) {
		return createBuffer(GL_PIXEL_PACK_BUFFER, size, data, usage);
	}
	static inline GLuint createArrayBuffer(unsigned size, const void *data, GLenum usage) {
		return createBuffer(GL_ARRAY_BUFFER, size, data, usage);
	}
	static inline GLuint createElementBuffer(unsigned size, const void *data, GLenum usage) {
		return createBuffer(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);
	}

	static GLuint createBuffer(GLenum target, GLuint size, const GLvoid* data, GLenum usage);
};

#endif	/* __GLBUFFERAGENT_H__ */