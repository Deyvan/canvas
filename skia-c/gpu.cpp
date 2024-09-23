#include "gpu.hpp"
#include <cstdio>

extern "C" {

    void gpu_init_volk(){
        volkInitialize();
    }

    void gpu_get_supported_devices(int *count, gpu_vk_device **devices) {

        if(count == nullptr) return;
        if(devices == nullptr) return;

        VkInstance instance;

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.apiVersion = VK_API_VERSION_1_0;
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pApplicationName = "listgpu";
        appInfo.pEngineName = "listgpu";
        appInfo.pNext = nullptr;

        VkInstanceCreateInfo instanceInfo{};
        instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceInfo.pApplicationInfo = &appInfo;
        instanceInfo.enabledExtensionCount = 0;
        instanceInfo.enabledLayerCount = 0;
        instanceInfo.flags = 0;
        instanceInfo.ppEnabledExtensionNames = nullptr;
        instanceInfo.ppEnabledLayerNames = nullptr;
        instanceInfo.pNext = nullptr;

        if (vkCreateInstance(&instanceInfo, nullptr, &instance) != VK_SUCCESS) {
            return;
        }

        volkLoadInstance(instance);

        uint32_t devicesCount = 0;
        if (vkEnumeratePhysicalDevices(instance, &devicesCount, nullptr) != VK_SUCCESS) {
            vkDestroyInstance(instance, nullptr);
            return;
        }

        if (devicesCount == 0) return;

        VkPhysicalDevice *physDevices = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * devicesCount);
        if (vkEnumeratePhysicalDevices(instance, &devicesCount, physDevices) != VK_SUCCESS) {
            free(physDevices);
            vkDestroyInstance(instance, nullptr);
            return;
        }

        *count = 0;
        *devices = (gpu_vk_device*)malloc(sizeof(gpu_vk_device) * devicesCount);

        for (uint32_t i = 0; i < devicesCount; i++) {

            bool under_ver_1_1 = false;

            VkPhysicalDeviceProperties prop;
            vkGetPhysicalDeviceProperties(physDevices[i], &prop);

            // skia support vulkan version 1.0
            // but need VK_KHR_dedicated_allocation and VK_KHR_get_memory_requirements2 gpu extensions
            if (prop.apiVersion < VK_MAKE_API_VERSION(0, 1, 1, 0)) {
                under_ver_1_1 = true;

                uint32_t extsCount;
                if (vkEnumerateDeviceExtensionProperties(physDevices[i], nullptr, &extsCount, nullptr) != VK_SUCCESS) {
                    continue;
                }

                VkExtensionProperties* exts = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * extsCount);
                if (vkEnumerateDeviceExtensionProperties(physDevices[i], nullptr, &extsCount, exts) != VK_SUCCESS) {
                    free(exts);
                    continue;
                }

                int requiredExtsCount = 0;

                for (int j = 0; j < extsCount; j++) {
                    if (strcmp(exts[j].extensionName, "VK_KHR_dedicated_allocation") == 0) {
                        requiredExtsCount++;
                    }else if (strcmp(exts[j].extensionName, "VK_KHR_get_memory_requirements2") == 0) {
                        requiredExtsCount++;
                    }
                }

                free(exts);

                if (requiredExtsCount < 2) continue;
            }

            uint32_t queueCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(physDevices[i], &queueCount, nullptr);

            VkQueueFamilyProperties* queues = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * queueCount);
            vkGetPhysicalDeviceQueueFamilyProperties(physDevices[i], &queueCount, queues);

            bool hasGraphicsQueue = false;
            for (int j = 0; j < queueCount; j++) {
                if (queues[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    hasGraphicsQueue = true;
                    break;
                }
            }

            free(queues);

            if (!hasGraphicsQueue) continue;

            devices[*count]->id = i;
            strcpy(devices[*count]->name, prop.deviceName);
            devices[*count]->type = prop.deviceType;
            devices[*count]->under_ver_1_1 = under_ver_1_1;

            (*count)++;
        }

        if (*count != 0) {
            *devices = (gpu_vk_device*)realloc(*devices, sizeof(gpu_vk_device) * (*count));
        }
        else {
            free(*devices);
            *devices = nullptr;
        }

        free(physDevices);
        vkDestroyInstance(instance, nullptr);
    }

}