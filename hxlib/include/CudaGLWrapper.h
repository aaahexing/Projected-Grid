#ifndef __CUDAGLWRAPPER_H__
#define __CUDAGLWRAPPER_H__

#include "CUDAWrapper.h"

#include <cassert>

#include "gl/glew.h"

class CudaGLContext {
public:
	CudaGLContext(CudaDevice &_device, int _res_count = 0);
	~CudaGLContext();

	inline CudaDevice& getDevice() {
		return m_device;
	}
	inline void mapResources() {
		cuGraphicsMapResources(m_resource_pc, m_resources, 0);
	}
	inline void mapResources(int _res_count) {
		assert(_res_count <= m_resource_pc);
		cuGraphicsMapResources(_res_count, m_resources, 0);
	}
	inline void unmapResources() {
		cuGraphicsMapResources(m_resource_pc, m_resources, 0);
	}
	inline void unmapResources(int _res_count) {
		assert(_res_count <= m_resource_pc);
		cuGraphicsUnmapResources(_res_count, m_resources, 0);
	}
	inline size_t getMappedDeviceBytes(int _res_index) {
		size_t size;
		CUdeviceptr ptr = NULL;
		assert(_res_index <= m_resource_pc);
		cuGraphicsResourceGetMappedPointer(&ptr, &size, m_resources[_res_index]);
		return size;
	}
	inline CUdeviceptr getMappedDevicePointer(int _res_index) {
		size_t size;
		CUdeviceptr ptr = NULL;
		assert(_res_index <= m_resource_pc);
		cuGraphicsResourceGetMappedPointer(&ptr, &size, m_resources[_res_index]);
		return ptr;
	}

	void registerBuffer(GLuint buffer, unsigned flags);
	void registerImage(GLuint image, GLenum target, unsigned flags);

protected:
	int m_resource_pc;
	int m_resource_cnt;
	CUcontext m_context;
	CudaDevice &m_device;
	CUgraphicsResource *m_resources;
};

#endif	/* __CUDAGLWRAPPER_H__ */