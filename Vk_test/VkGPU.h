#pragma once

#include "K3Vk.h"

class VkGPU {

public:
	
	void						init(VkInstance const& instance, VkSurfaceKHR const& surface);
	static						SwapChainSupportDetails		querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR const& surface);
	VkDevice const&				getLogicalDevice() const;
	VkPhysicalDevice const&		getPhysicalDevice() const;
	VkQueue const&				getGfxQueue() const;
	VkQueue const&				getTransferQueue() const;
	VkQueue const&				getPresentQueue() const;
	VkQueue const&				getComputeQueue() const;
	uint32_t const*				getQueuesIndex() const;


	VkGPU(VkInstance const& instance, VkSurfaceKHR const& surface) {
		init(instance, surface);
	}

	~VkGPU() {}

private:

	//FUNCTIONS
	void						findPhysicalDevice(VkInstance const& instance, VkSurfaceKHR const& surface);
	void						createLogicalDevice();
	bool						findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR const& surface);
	bool						isSuitableDevice(VkPhysicalDevice device, VkSurfaceKHR const& surface);
	bool						checkExtensionSupport(VkPhysicalDevice device);
	void						getQueues();

	// VARIABLES
	VkPhysicalDevice				physicalDevice = VK_NULL_HANDLE;
	VkDevice						logicalDevice = VK_NULL_HANDLE;
	VkInstance						instance;
	VkQueue							gfxQueue;
	VkQueue							presentQueue;
	VkQueue							transferQueue;
	VkQueue							computeQueue;
	uint32_t						queuesIndex[NB_QUEUES];

};