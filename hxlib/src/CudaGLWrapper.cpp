#include "CudaGLWrapper.h"

#include <cudaGL.h>

CudaGLContext::CudaGLContext(CudaDevice &_device, int _res_count /* = 0 */)
	: m_device(_device), m_resource_cnt(_res_count) {
	CUresult err;
	if ((err = cuGLCtxCreate(&m_context, 0, _device.getDeviceID())) != CUDA_SUCCESS) {
		fprintf(stderr, "Failed to create CUDA context of OpenGL interoperability, error code %d\n", err);
	}
	m_resource_pc = 0;
	m_resources = new CUgraphicsResource[_res_count];
}

CudaGLContext::~CudaGLContext() {
	CUresult err;
	if ((err = cuCtxDetach(m_context)) != CUDA_SUCCESS) {
		fprintf(stderr, "Failed to detach CUDA context of OpenGL interoperability, error code %d\n", err);
	}
	delete [] m_resources;
}

void CudaGLContext::registerBuffer(GLuint buffer, unsigned flags) {
	CUresult err;
	if ((err = cuGraphicsGLRegisterBuffer(&m_resources[m_resource_pc], buffer, flags)) != CUDA_SUCCESS) {
		fprintf(stderr, "Failed to register OpenGL buffer object with CUDA, error code = %d\n", err);
	}
	++m_resource_pc;
	assert(m_resource_pc <= m_resource_cnt);
}

void CudaGLContext::registerImage(GLuint image, GLenum target, unsigned flags) {
	CUresult err;
	if ((err = cuGraphicsGLRegisterImage(&m_resources[m_resource_pc], image, target, flags)) != CUDA_SUCCESS) {
		fprintf(stderr, "Failed to register OpenGL image with CUDA, error code  = %d\n", err);
	}
	++m_resource_pc;
	assert(m_resource_pc <= m_resource_cnt);
}
