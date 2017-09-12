#pragma once

#include "K3Vk.h"
#include "VkGPU.h"

class VkDisplayHandler {

public:

	void						initWindow();
	void						createSurface(VkInstance const& instance);
	void						destroySurface(VkInstance const& instance) const;
	void						terminateWindow();
	void						createSwapchain(VkSwapchainKHR oldSC, VkPhysicalDevice const& gpuPDevice, VkDevice const& gpuLDevice);
	void						destroySwapchain(VkDevice const& gpuDev) const;
	void						createImgViews(VkDevice gpuDevice);
	void						destroyImgViews(VkDevice const& gpuDev) const;
	void						createFrameBuffers(VkDevice gpuDevice, VkRenderPass renderPass);
	void						destroyFramebuffers(VkDevice const& gpuDev) const;
	VkSurfaceKHR const&			getSurface() const;
	VkFormat const&				getScImgFormat() const;
	VkExtent2D const&			getScExtent() const;
	GLFWwindow* const&			getWindow() const;
	VkSwapchainKHR const&		getSwapchain() const;
	std::vector<VkFramebuffer> const&	getFramebuffers() const;
	void						resizeWindow(uint32_t const newSizeX, uint32_t const newSizeY, bool const fullscreen);

	VkDisplayHandler() {
		initWindow();
	}
	
	~VkDisplayHandler() {}

private:

	VkSurfaceFormatKHR			pickSCSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
	VkPresentModeKHR			pickSCPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
	VkExtent2D					pickSCExtent(const VkSurfaceCapabilitiesKHR& capabilities);


	uint32_t						windowWidth = 800;
	uint32_t						windowHeight = 600;
	GLFWwindow						*window;
	VkSurfaceKHR					surface;
	VkSwapchainKHR					swapchain;
	std::vector<VkImage>			scImages;
	std::vector<VkImageView>		scImgView;
	VkFormat						scImgFormat;
	VkExtent2D						scExtent;
	std::vector<VkFramebuffer>		scFramebuffers;

};