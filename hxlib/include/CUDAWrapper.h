#ifndef __CUDAWRAPPER_H__
#define __CUDAWRAPPER_H__

/*!
	An simple wrapper of CUDA functions in common use.
*/

#include <string>

#include <cuda.h>

class CudaDriverAgent {
public:
	static void init();
	static int getDeviceCount();
	static int getDriverVersion();
};

class CudaDevice {
public:
	CudaDevice(int _device_id);

	inline int getMaxGridX() {
		return getAttribute(CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_X);
	}
	inline int getMaxGridY() {
		return getAttribute(CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Y);
	}
	inline int getMaxGridZ() {
		return getAttribute(CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Z);
	}
	inline int getMaxBlockX() {
		return getAttribute(CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_X);
	}
	inline int getMaxBlockY() {
		return getAttribute(CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Y);
	}
	inline int getMaxBlockZ() {
		return getAttribute(CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Z);
	}
	inline int getWarpSize() {
		return getAttribute(CU_DEVICE_ATTRIBUTE_WARP_SIZE);
	}
	inline int getMaxThreadsPerBlock() {
		return getAttribute(CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_BLOCK);
	}
	inline int getMaxRegistersPerBlock() {
		return getAttribute(CU_DEVICE_ATTRIBUTE_MAX_REGISTERS_PER_BLOCK);
	}
	inline int getDeviceID() {
		return m_device_id;
	}

	int getMajorVersion();
	int getMinorVersion();
	std::string getName();
	size_t getTotalMemory();
	int getAttribute(CUdevice_attribute attrib);

protected:
	int m_device_id;
	CUdevice m_device;
};

#endif	/* __CUDAWRAPPER_H__ */