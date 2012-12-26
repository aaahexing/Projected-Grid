#include "CUDAWrapper.h"

#include <cstdio>

#include <cuda.h>

void CudaDriverAgent::init() {
	CUresult err;
	if ((err = cuInit(0)) != CUDA_SUCCESS) {
		fprintf(stderr, "Failed to initialize CUDA driver, error code %d\n", err);
	}
}

int CudaDriverAgent::getDeviceCount() {
	int count;
	CUresult err;
	if ((err = cuDeviceGetCount(&count)) != CUDA_SUCCESS) {
		fprintf(stderr, "Failed to get CUDA device count, error code %d\n", err);
	}
	return count;
}

int CudaDriverAgent::getDriverVersion() {
	int version;
	CUresult err;
	if ((err = cuDriverGetVersion(&version)) != CUDA_SUCCESS) {
		fprintf(stderr, "Failed to get CUDA driver version, error code %d\n", err);
	}
	return version;
}

CudaDevice::CudaDevice(int _device_id) 
	: m_device_id(_device_id) {
	CUresult err;
	if ((err = cuDeviceGet(&m_device, _device_id)) != CUDA_SUCCESS) {
		fprintf(stderr, "Failed to open CUDA device, error code %d\n", err);
	}
}

int CudaDevice::getMajorVersion() {
	int maj, min;
	CUresult err;
	if ((err = cuDeviceComputeCapability(&maj, &min, m_device)) != CUDA_SUCCESS) {
		fprintf(stderr, "Failed to get CUDA device version, error code %d\n", err);
	}
	return maj;
}

int CudaDevice::getMinorVersion() {
	int maj, min;
	CUresult err;
	if ((err = cuDeviceComputeCapability(&maj, &min, m_device)) != CUDA_SUCCESS) {
		fprintf(stderr, "Failed to get CUDA device version, error code %d\n", err);
	}
	return min;
}

std::string CudaDevice::getName() {
	char buf[256];
	CUresult err;
	if ((err = cuDeviceGetName(buf, 256, m_device)) != CUDA_SUCCESS) {
		fprintf(stderr, "Failed to get CUDA device name, error code %d\n", err);
	}
	return std::string(buf);
}

size_t CudaDevice::getTotalMemory() {
	size_t total;
	CUresult err;
	if ((err = cuDeviceTotalMem(&total, m_device)) != CUDA_SUCCESS) {
		fprintf(stderr, "Failed to get CUDA total memory, error code %d\n", err);
	}
	return total;
}

int CudaDevice::getAttribute(CUdevice_attribute attrib) {
	int ret;
	CUresult err;
	if ((err = cuDeviceGetAttribute(&ret, attrib, m_device)) != CUDA_SUCCESS) {
		fprintf(stderr, "Failed to get CUDA device attribute, error code %d\n", m_device);
	}
	return ret;
}

