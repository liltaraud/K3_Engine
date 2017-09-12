#include "VkGPU.h"


void	VkGPU::init(VkInstance const& instance, VkSurfaceKHR const& surface)
{
	if (instance == VK_NULL_HANDLE || !surface)
		throw std::exception("Objects needed for the creation of a VkGPU object are not valid !");
	findPhysicalDevice(instance, surface);
	createLogicalDevice();
	getQueues();
}

void		VkGPU::findPhysicalDevice(VkInstance const& instance, VkSurfaceKHR const& surface)
{
	uint32_t	deviceCount = 0;

	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	if (deviceCount == 0)
		throw std::runtime_error("Failed to find any GPUs with Vulkan support !");
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
	for (const auto& dev : devices) {
		if (isSuitableDevice(dev, surface)) {
			physicalDevice = dev;
			break;
		}
	}
	if (physicalDevice == VK_NULL_HANDLE)
		throw std::runtime_error("Failed to find a suitable GPU");
}

bool		VkGPU::checkExtensionSupport(VkPhysicalDevice device)
{
	uint32_t	extensionCount;

	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
	if (extensionCount < 1)
		return false;
	std::vector<VkExtensionProperties>		deviceExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, deviceExtensions.data());

	std::set<std::string>	extensionList(requiredExtensions.begin(), requiredExtensions.end());
	for (const auto& extension : deviceExtensions) {
		extensionList.erase(extension.extensionName);
	}
	return (extensionList.empty());
}

SwapChainSupportDetails		VkGPU::querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR const& surface)
{
	uint32_t					formatCount;
	uint32_t					presentModesCount;
	SwapChainSupportDetails		scDetails;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &scDetails.capabilities);

	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
	if (formatCount > 0) {
		scDetails.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, scDetails.formats.data());
	}

	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModesCount, nullptr);
	if (presentModesCount > 1) {
		scDetails.presentModes.resize(presentModesCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModesCount, scDetails.presentModes.data());
	}

	return scDetails;
}

VkDevice const & VkGPU::getLogicalDevice() const
{
	return logicalDevice;
}

VkPhysicalDevice const & VkGPU::getPhysicalDevice() const
{
	return physicalDevice;
}

VkQueue const & VkGPU::getGfxQueue() const
{
	return gfxQueue;
}

VkQueue const & VkGPU::getTransferQueue() const
{
	return transferQueue;
}

VkQueue const & VkGPU::getPresentQueue() const
{
	return presentQueue;
}

VkQueue const & VkGPU::getComputeQueue() const
{
	return computeQueue;
}

uint32_t const*		VkGPU::getQueuesIndex() const
{
	return queuesIndex;
}

bool		VkGPU::isSuitableDevice(VkPhysicalDevice device, VkSurfaceKHR const& surface)
{
	VkPhysicalDeviceProperties			deviceProperties;
	VkPhysicalDeviceFeatures			deviceFeatures;
	SwapChainSupportDetails				scDetails = {};

	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
	scDetails = querySwapChainSupport(device, surface);
	if (deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
		|| !deviceFeatures.geometryShader || !findQueueFamilies(device, surface)
		|| !checkExtensionSupport(device) || scDetails.formats.empty()
		|| scDetails.presentModes.empty())
		return false;
	return true;
}

bool		VkGPU::findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR const& surface)
{
	uint32_t					queueCount;
	bool						queueFound = false;
	std::vector<VkQueueFlags>	queueFlags(3);
	int							qidx = 1;
	uint32_t					idx;

	// Required queues flags ///////////////
	queueFlags[0] = VK_QUEUE_GRAPHICS_BIT;
	queueFlags[1] = VK_QUEUE_COMPUTE_BIT;
	queueFlags[2] = VK_QUEUE_TRANSFER_BIT;

	////////////////////////////////////////
	memset(queuesIndex, -1, NB_QUEUES);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, nullptr);
	if (!queueCount)
		return false;
	std::vector<VkQueueFamilyProperties>	deviceQueues(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, deviceQueues.data());
	for (const auto& flags : queueFlags) {
		idx = 0;
		for (const auto& queues : deviceQueues) {
			if (queues.queueCount > 0 && queues.queueFlags & flags) {
				if (!(flags == VK_QUEUE_TRANSFER_BIT && queues.queueFlags & VK_QUEUE_GRAPHICS_BIT))
				{
					queueFound = true;
					queuesIndex[qidx] = idx;
					break;
				}
			}
			idx++;
		}
		if (!queueFound)
			return false;
		queueFound = false;
		qidx++;
	}

	VkBool32					presentationSupport = false;

	idx = 0;
	for (const auto& queues : deviceQueues) {
		vkGetPhysicalDeviceSurfaceSupportKHR(device, idx, surface, &presentationSupport);
		if (presentationSupport) {
			queuesIndex[0] = idx;
			break;
		}
		idx++;
	}
	if (!presentationSupport)
		return false;
	return true;
}


void		VkGPU::createLogicalDevice()
{
	std::vector<VkDeviceQueueCreateInfo> queuesInfo;
	std::set<uint32_t>	queueFamilies = { queuesIndex[0], queuesIndex[1], queuesIndex[2], queuesIndex[3] };
	float					queuePriority = 1.0f;

	for (uint32_t currentQueueFam : queueFamilies) {
		VkDeviceQueueCreateInfo		queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = currentQueueFam;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queuesInfo.push_back(queueCreateInfo);
	}
	VkPhysicalDeviceFeatures deviceFeatures = {};
	VkDeviceCreateInfo		deviceInfo = {};

	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queuesInfo.size());
	deviceInfo.pQueueCreateInfos = queuesInfo.data();
	deviceInfo.pEnabledFeatures = &deviceFeatures;
	deviceInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
	deviceInfo.ppEnabledExtensionNames = requiredExtensions.data();

	if (enableValidationLayers) {
		deviceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		deviceInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
		deviceInfo.enabledLayerCount = 0;

	if (vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &logicalDevice) != VK_SUCCESS)
		throw std::exception("Failed to create logical device");
}

void			VkGPU::getQueues()
{
	vkGetDeviceQueue(logicalDevice, queuesIndex[0], 0, &presentQueue);
	vkGetDeviceQueue(logicalDevice, queuesIndex[1], 0, &gfxQueue);
	vkGetDeviceQueue(logicalDevice, queuesIndex[2], 0, &computeQueue);
	vkGetDeviceQueue(logicalDevice, queuesIndex[3], 0, &transferQueue);
}