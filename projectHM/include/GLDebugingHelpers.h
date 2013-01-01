#pragma once

#ifndef __GLDEBUGGINGHELPERS_H__
#define __GLDEBUGGINGHELPERS_H__

#include <cstdio>

#include "gl/glew.h"
#include "gl/glut.h"

inline void showVec3(const float* vec) {
	for (int i = 0; i < 3; ++i) {
		printf("%.3f ", vec[i]);
	}
	puts("");
}

inline void showMat4(const float* mat) {
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			printf("%.3f ", mat[i * 4 + j]);
		}
		puts("");
	}
}

inline void showModelViewMat() {
	GLfloat model_view_mat[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, model_view_mat);
	puts("model-view-matrix:");
	showMat4(model_view_mat);
}

inline void showProjectionMat() {
	GLfloat projection_mat[16];
	glGetFloatv(GL_PROJECTION_MATRIX, projection_mat);
	puts("projection-matrix:");
	showMat4(projection_mat);
}

#endif