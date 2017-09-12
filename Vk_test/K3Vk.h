#pragma once

# define VK_USE_PLATFORM_WIN32_KHR
# include <vulkan/vulkan.h>
# define GLFW_INCLUDE_VULKAN
# define GLFW_EXPOSE_NATIVE_WIN32
# include <GLFW/glfw3.h>
# include <GLFW/glfw3native.h>
# define GLM_FORCE_RADIANS
# define GLM_FORCE_DEPTH_ZERO_TO_ONE
# include <glm/glm.hpp>
# include <glm/vec4.hpp>
# include <glm/mat4x4.hpp>
# include <stdexcept>
# include <functional>
# include <iostream>
# include <vector>
# include <set>
# include <string>
# include <algorithm>
# include <fstream>
# include <array>

#define NB_QUEUES 4

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const std::vector<const char *>	validationLayers = {
	"VK_LAYER_LUNARG_standard_validation",
	"VK_LAYER_LUNARG_object_tracker"
};

const std::vector<const char *>	requiredExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR		capabilities;
	std::vector<VkSurfaceFormatKHR>	formats;
	std::vector<VkPresentModeKHR>	presentModes;
};