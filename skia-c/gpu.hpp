#ifndef SKIA_GPUAPI_H
#define SKIA_GPUAPI_H

#include <volk.h>
#include <memory>

struct gpu_vk_device {
	uint32_t id;
	VkPhysicalDeviceType type;
	char name[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE];

	bool under_ver_1_1; // internal
};

extern "C" {

    void gpu_init_volk();
    void gpu_get_supported_devices(int *count, gpu_vk_device **devices);

}

#endif // SKIA_GPUAPI_H
