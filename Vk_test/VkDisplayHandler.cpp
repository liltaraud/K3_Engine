#include "VkDisplayHandler.h"


void				VkDisplayHandler::initWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(windowWidth, windowHeight, "K3 Engine v0.0.1", nullptr, nullptr);
}


void				VkDisplayHandler::createSurface(VkInstance const& instance)
{
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create window surface !");
	}
}

void VkDisplayHandler::destroySurface(VkInstance const & instance) const
{
	vkDestroySurfaceKHR(instance, surface, nullptr);
}

VkExtent2D			VkDisplayHandler::pickSCExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	VkExtent2D	newExtent;

	newExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, windowWidth));
	newExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.width, windowHeight));

	return newExtent;
}

VkPresentModeKHR	VkDisplayHandler::pickSCPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes)
{
	// Add configuration options for this in the future

	return VK_PRESENT_MODE_IMMEDIATE_KHR;
}

VkSurfaceFormatKHR	VkDisplayHandler::pickSCSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
	VkSurfaceFormatKHR		preferredFormat = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
		return preferredFormat;
	}
	for (const auto& formats : availableFormats) {
		if (formats.format == preferredFormat.format && formats.colorSpace == preferredFormat.colorSpace) {
			return preferredFormat;
		}
	}

	return availableFormats[0];
}

void				VkDisplayHandler::createSwapchain(VkSwapchainKHR oldSC, VkPhysicalDevice const& gpuPDevice, VkDevice const& gpuLDevice)
{
	SwapChainSupportDetails scInfo = VkGPU::querySwapChainSupport(gpuPDevice, surface);
	uint32_t				imgCount = scInfo.capabilities.minImageCount;
	uint32_t				maxImgCount = scInfo.capabilities.maxImageCount;
	VkSurfaceFormatKHR		scSurfaceFormat = pickSCSurfaceFormat(scInfo.formats);
	VkPresentModeKHR		scPresent = pickSCPresentMode(scInfo.presentModes);

	scExtent = pickSCExtent(scInfo.capabilities);
	scImgFormat = scSurfaceFormat.format;

	if (scPresent == VK_PRESENT_MODE_MAILBOX_KHR)
		imgCount++;
	if (maxImgCount > 0 && imgCount > maxImgCount)
		imgCount = maxImgCount;

	VkSwapchainCreateInfoKHR	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imgCount;
	createInfo.imageFormat = scSurfaceFormat.format;
	createInfo.imageColorSpace = scSurfaceFormat.colorSpace;
	createInfo.imageExtent = scExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	// ^ I need to temporarily release ownership of my images to use them with another queue

	createInfo.preTransform = scInfo.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = scPresent;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = oldSC;

	if (vkCreateSwapchainKHR(gpuLDevice, &createInfo, nullptr, &swapchain) != VK_SUCCESS)
		throw std::runtime_error("Swap Chain creation failed !");
	if (oldSC != VK_NULL_HANDLE)
		vkDestroySwapchainKHR(gpuLDevice, oldSC, nullptr);
}

void VkDisplayHandler::destroySwapchain(VkDevice const& gpuDev) const
{
	vkDestroySwapchainKHR(gpuDev, swapchain, nullptr);
}


void				VkDisplayHandler::createImgViews(VkDevice gpuDevice)
{
	uint32_t		imgCount;

	vkGetSwapchainImagesKHR(gpuDevice, swapchain, &imgCount, nullptr);
	scImages.resize(imgCount);
	vkGetSwapchainImagesKHR(gpuDevice, swapchain, &imgCount, scImages.data());
	scImgView.resize(scImages.size());

	for (size_t i = 0; i < scImages.size(); i++) {
		VkImageViewCreateInfo		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = scImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = scImgFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(gpuDevice, &createInfo, nullptr, &scImgView[i]))
			throw std::runtime_error("Failed to create image view !");
	}
}

void				VkDisplayHandler::createFrameBuffers(VkDevice gpuDevice, VkRenderPass renderPass)
{
	scFramebuffers.resize(scImgView.size());

	for (size_t i = 0; i < scImgView.size(); i++) {
		VkImageView		attachments[] = {
			scImgView[i]
		};

		VkFramebufferCreateInfo		fbInfo = {};
		fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbInfo.renderPass = renderPass;
		fbInfo.attachmentCount = 1;
		fbInfo.pAttachments = attachments;
		fbInfo.width = scExtent.width;
		fbInfo.height = scExtent.height;
		fbInfo.layers = 1;

		if (vkCreateFramebuffer(gpuDevice, &fbInfo, nullptr, &scFramebuffers[i]) != VK_SUCCESS)
			throw std::runtime_error("failed to create framebuffer!");
	}
}

VkSurfaceKHR const&	VkDisplayHandler::getSurface() const
{
	return surface;
}

VkFormat const&	VkDisplayHandler::getScImgFormat() const
{
	return scImgFormat;
}

VkExtent2D const&	VkDisplayHandler::getScExtent() const
{
	return scExtent;
}

GLFWwindow* const&	VkDisplayHandler::getWindow() const
{
	return window;
}

VkSwapchainKHR const&	VkDisplayHandler::getSwapchain() const
{
	return swapchain;
}

std::vector<VkFramebuffer> const&	VkDisplayHandler::getFramebuffers() const
{
	return scFramebuffers;
}


/* Sets the screen to the new sizes given as parameter, or sets the window to fullscreen
** default monitor resolution if the fullscreen parameter is set to 1
*/

void VkDisplayHandler::resizeWindow(uint32_t const newSizeX, uint32_t const newSizeY, bool const fullscreen)
{
	if ((newSizeX < 1 || newSizeY < 1) && !fullscreen)
		return;
	if (!window) {
		throw std::runtime_error("Window specified doesn't exist !");
		return;
	}
	
	GLFWmonitor*		monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode*	mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	
	if (!mode)
		throw std::runtime_error("An error occured while retrieving monitor information !");
	if (fullscreen) {
		windowWidth = mode->width;
		windowHeight = mode->height;
		glfwSetWindowMonitor(window, monitor, 0, 0, windowWidth, windowHeight, mode->refreshRate);
		return;
	}
	glfwSetWindowSize(window, newSizeX, newSizeY);
	glfwSetWindowPos(window, (mode->width - newSizeX) / 2, (mode->height - newSizeY) / 2);
	windowWidth = newSizeX;
	windowHeight = newSizeY;
}

void VkDisplayHandler::destroyImgViews(VkDevice const& gpuDev) const
{
	for (size_t i = 0; i < scImgView.size(); i++) {
		vkDestroyImageView(gpuDev, scImgView[i], nullptr);
	}
}

void VkDisplayHandler::destroyFramebuffers(VkDevice const & gpuDev) const
{
	for (size_t i = 0; i < scFramebuffers.size(); i++) {
		vkDestroyFramebuffer(gpuDev, scFramebuffers[i], nullptr);
	}
}

void				VkDisplayHandler::terminateWindow()
{
	if (window) {
		glfwDestroyWindow(window);
	}
	glfwTerminate();
}