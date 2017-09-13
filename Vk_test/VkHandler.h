#pragma once

#include "K3Vk.h"
#include "VkGPU.h"
#include "VkDisplayHandler.h"
#define NB_QUEUES 4

struct Vertex
{
	glm::vec2	pos;
	glm::vec3	color;

	static		VkVertexInputBindingDescription		getBindingDescription() {
		VkVertexInputBindingDescription	bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	static		std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
		
		std::array<VkVertexInputAttributeDescription, 2>		attributeDescriptions = {};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);
		return attributeDescriptions;
	}
};

const std::vector<Vertex> vertices = {
	{ { -0.5f, -0.5f },{ 1.0f, 0.0f, 0.0f } },
	{ { 0.5f, -0.5f },{ 0.0f, 1.0f, 0.0f } },
	{ { 0.5f, 0.5f },{ 0.0f, 0.0f, 1.0f } },
	{ { -0.5f, 0.5f },{ 1.0f, 1.0f, 1.0f } }
};

const std::vector<uint16_t>		indices = {
	0, 1, 2, 2, 3, 0
};

class VkHandler {
	friend class VkGPU;

public:
	void				run();
	void				terminate();
	void				resizeWindow(const int newSizeX, const int newSizeY, const bool fullscreen);
	static uint32_t			findMemoryType(VkPhysicalDevice physicalGPU, uint32_t typeFilter, VkMemoryPropertyFlags properties);

	VkHandler() {
		initSubClasses();
	}
	~VkHandler() {
		delete dispHandler;
		delete gpu;
	}

private:
	
	void				initSubClasses();
	void				initVulkan();
	void				mainLoop();
	void				createInstance();
	bool				checkValidationLayerSupport();
	std::vector<const char *>	getGlfwRequiredExtensions();
	void				setupDebugCallback();
	void				terminateVulkan();
	char*				getMissingQueue(VkQueueFlags);
	void				createRenderPass();
	void				createGFXPipeline();
	void				createCmdPool();
	void				createCmdBuffers();
	void				createSemaphores();
	void				createVertexBuffer();
	void				createIndexBuffer();
	void				copyBuffer(VkBuffer src, VkBuffer dst, VkBufferCopy *copyInfo, uint32_t copyInfoSize, VkFence fence);
	void				createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProperties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void				recreateSwapChain();
	void				cleanupSwapChainAssets();
	void				drawFrame();
	VkShaderModule			createShaderModuleFromSrc(const std::string& filename);
	void				DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator);
	VkResult			CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);
	void				transferBufferToGpuStaged(void const* bufferDataVkBuffer, VkDeviceSize const bufferDataSize, VkBuffer& dstBuffer, VkDeviceMemory& dstBufferMemory, int const copySrcOffst, int const copyDstOffst);
	
	////////////////////////////////////
	// VARIABLES
	////////////////////////////////////


	VkInstance			instance;
	VkDebugReportCallbackEXT	callback;
	VkDisplayHandler		*dispHandler;
	VkGPU				*gpu;
	VkRenderPass			renderPass;
	VkPipelineLayout		pipelineLayout;
	VkPipeline			gfxPipeline;
	VkCommandPool			cmdPools[4];
	std::vector<VkCommandBuffer>	cmdBuffers;
	VkSemaphore			semImgAvailable;
	VkSemaphore			semRenderFinished;
	VkBuffer			vertexBuffer;
	VkDeviceMemory			vertexBufferMemory;
	uint32_t			vertexObjectSize;
	VkBuffer			indexBuffer;
	VkDeviceMemory			indexBufferMemory;

};
