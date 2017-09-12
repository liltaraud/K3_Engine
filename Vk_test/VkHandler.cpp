#include "VkHandler.h"

static		VKAPI_ATTR VkBool32 VKAPI_CALL validationLayerCallback(
	VkDebugReportFlagsEXT		Flags,
	VkDebugReportObjectTypeEXT	objType,
	uint64_t					obj,
	size_t						location,
	int32_t						code,
	const char*					layerPrefix,
	const char*					msg,
	void*						userData)
{
	std::cerr << "Validation layer callback : " << msg << std::endl;
	return VK_FALSE;
}

std::vector<const char *>	VkHandler::getGlfwRequiredExtensions()
{
	std::vector<const char *>	extensions;
	unsigned int				glfwExtensionCount = 0;
	const char					**glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	
	for (unsigned int i = 0; i < glfwExtensionCount; i++) {
		extensions.push_back(glfwExtensions[i]);
	}

	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}
	return (extensions);
}

bool			VkHandler::checkValidationLayerSupport()
{
	uint32_t		layerCount;
	bool			layerFound;

	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties>	availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	std::cout << "Available validation layers :" << std::endl;
	for (const auto &layers : availableLayers) {
		std::cout << "-" << layers.layerName << std::endl;
	}

	for (const char* layerName : validationLayers) {
		layerFound = false;
		for (const auto &layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}
		if (!layerFound)
			return false;
	}
	return true;
}

void		VkHandler::initSubClasses()
{
	dispHandler = new VkDisplayHandler;
	createInstance();
	if (enableValidationLayers)
		setupDebugCallback();
	dispHandler->createSurface(instance);
	gpu = new VkGPU(instance, dispHandler->getSurface());
}

void		VkHandler::run()
{
	initVulkan();
	mainLoop();
}

void		VkHandler::terminate()
{
	dispHandler->terminateWindow();
	terminateVulkan();
}

VkResult	VkHandler::CreateDebugReportCallbackEXT(
	VkInstance									instance,
	const VkDebugReportCallbackCreateInfoEXT*	pCreateInfo,
	const VkAllocationCallbacks*				pAllocator,
	VkDebugReportCallbackEXT*					pCallback)
{
	auto func = (PFN_vkCreateDebugReportCallbackEXT)
		vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if (func != nullptr)
		return func(instance, pCreateInfo, pAllocator, pCallback);
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void		VkHandler::DestroyDebugReportCallbackEXT(
	VkInstance									instance,
	VkDebugReportCallbackEXT					callback,
	const VkAllocationCallbacks*				pAllocator)
{
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)
		vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	if (func != nullptr)
		func(instance, callback, pAllocator);
}

void		VkHandler::setupDebugCallback()
{
	VkDebugReportCallbackCreateInfoEXT callbackInfo = {};
	callbackInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	callbackInfo.flags = VK_DEBUG_REPORT_DEBUG_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT;
	callbackInfo.pfnCallback = validationLayerCallback;
	if (CreateDebugReportCallbackEXT(instance, &callbackInfo, nullptr, &callback) != VK_SUCCESS) {
		throw std::runtime_error("Failed to set up validation layer callback function !");
	}
}

void		VkHandler::createInstance()
{
	// Specify the instance parameters

	if (enableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("Validation layers requested are not supported !");
	}

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "K3 Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// Get The extensions required to interface with the GLFW Window
	auto glfwExtensions = getGlfwRequiredExtensions();

	createInfo.enabledExtensionCount = static_cast<uint32_t>(glfwExtensions.size());
	createInfo.ppEnabledExtensionNames = glfwExtensions.data();

	// Get the number of supported validation layers and specify them
	if (enableValidationLayers) { // Number of validation layers wanted
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	// Create the instance with the specified parameters
	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create instance !");
	}
}

char*	VkHandler::getMissingQueue(VkQueueFlags flag)
{
	switch (flag)
	{
	case (0x00000001):
			return ("VK_QUEUE_GRAPHICS_BIT is missing !");
	case (0x00000002):
			return ("VK_QUEUE_COMPUTE_BIT is missing !");
	case (0x00000004):
		return ("VK_QUEUE_TRANSFER_BIT is missing !");
	}
	return ("VK_QUEUE_SPARSE_BINDING_BIT is missing !");
}

void			VkHandler::createRenderPass()
{
	VkAttachmentDescription		colorAttDes = {};
	colorAttDes.format = dispHandler->getScImgFormat();
	colorAttDes.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttDes.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttDes.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttDes.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttDes.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	colorAttDes.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;	// Z-BUFFER ATTACHMENT CONFIGURATION
	colorAttDes.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;	// Z-BUFFER ATTACHMENT CONFIGURATION

	VkAttachmentReference		colorAttRef = {};
	colorAttRef.attachment = 0;
	colorAttRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//SUBPASS CREATION
	VkSubpassDescription		subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttRef;
	subpass.pDepthStencilAttachment = nullptr; // Z BUFFER ATTACHMENTS

	// SUBPASS DEPENDENCIES
	VkSubpassDependency		dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;


	VkRenderPassCreateInfo		renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttDes;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(gpu->getLogicalDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
		throw std::runtime_error("Failed to create render pass !");
}

void			VkHandler::createGFXPipeline()
{
	VkShaderModule	vertShaderModule = createShaderModuleFromSrc("G:/Graphic_Projects/Vulkan/k3_engine/Vk_test/shaders/shader.vert.spv");
	VkShaderModule	fragShaderModule = createShaderModuleFromSrc("G:/Graphic_Projects/Vulkan/k3_engine/Vk_test/shaders/shader.frag.spv");

	// VERTEX SHADER
	VkPipelineShaderStageCreateInfo	vertStageInfo = {};
	vertStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertStageInfo.module = vertShaderModule;
	vertStageInfo.pName = "main";

	// FRAGMENT SHADER
	VkPipelineShaderStageCreateInfo	fragStageInfo = {};
	fragStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragStageInfo.module = fragShaderModule;
	fragStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo	shaderStages[] = {
		vertStageInfo,
		fragStageInfo
	};

	// VERTICES

	auto	bindingDescription = Vertex::getBindingDescription();
	auto	attributeDescriptions = Vertex::getAttributeDescriptions();

	// VERTEX INPUT SHADER
	VkPipelineVertexInputStateCreateInfo	vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	// VERTEX ASSEMBLY SHADER
	VkPipelineInputAssemblyStateCreateInfo	inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// SCISSOR
	VkRect2D		scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = dispHandler->getScExtent();

	// VIEWPORT
	VkViewport		vp = {};
	vp.x = 0.0f;
	vp.y = 0.0f;
	vp.width = static_cast<float>(scissor.extent.width);
	vp.height = static_cast<float>(scissor.extent.height);
	vp.minDepth = 0.0f;
	vp.maxDepth = 1.0f;



	// VIEWPORT AND SCISSOR STATE
	VkPipelineViewportStateCreateInfo	vpState = {};
	vpState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vpState.viewportCount = 1;
	vpState.pViewports = &vp;
	vpState.scissorCount = 1;
	vpState.pScissors = &scissor;

	// RASTERIZER
	VkPipelineRasterizationStateCreateInfo	rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	// MULTISAMPLING
	VkPipelineMultisampleStateCreateInfo	multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;

	// Z-BUFFER (for later)
	// VkPipelineDepthStencilStateCreateInfo

	// COLOR BLENDING
	VkPipelineColorBlendAttachmentState		colorBlendAttach = {};
	colorBlendAttach.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
								VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttach.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo		colorBlendInfo = {};
	colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendInfo.logicOpEnable = VK_FALSE;
	colorBlendInfo.attachmentCount = 1;
	colorBlendInfo.pAttachments = &colorBlendAttach;

	// DYNAMIC STATE
	VkDynamicState		dSList[] = {
		VK_DYNAMIC_STATE_LINE_WIDTH,
		VK_DYNAMIC_STATE_VIEWPORT
	};
	VkPipelineDynamicStateCreateInfo	dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dSList;

	// PIPELINE LAYOUT
	VkPipelineLayoutCreateInfo		pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = 0;
	if (vkCreatePipelineLayout(gpu->getLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		throw std::runtime_error("Failed to create pipeline layout object !");

	//CREATE GFX PIPELINE
	VkGraphicsPipelineCreateInfo	gfxPipelineInfo = {};
	gfxPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	gfxPipelineInfo.stageCount = 2;
	gfxPipelineInfo.pStages = shaderStages;
	gfxPipelineInfo.pVertexInputState = &vertexInputInfo;
	gfxPipelineInfo.pInputAssemblyState = &inputAssembly;
	gfxPipelineInfo.pViewportState = &vpState;
	gfxPipelineInfo.pRasterizationState = &rasterizer;
	gfxPipelineInfo.pMultisampleState = &multisampling;
	gfxPipelineInfo.pDepthStencilState = nullptr;
	gfxPipelineInfo.pColorBlendState = &colorBlendInfo;
	gfxPipelineInfo.pDynamicState = nullptr;
	gfxPipelineInfo.layout = pipelineLayout;
	gfxPipelineInfo.renderPass = renderPass;
	gfxPipelineInfo.subpass = 0;
	gfxPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	gfxPipelineInfo.basePipelineIndex = -1;

	if (vkCreateGraphicsPipelines(gpu->getLogicalDevice(), VK_NULL_HANDLE, 1, &gfxPipelineInfo, nullptr, &gfxPipeline) != VK_SUCCESS)
		throw std::runtime_error("failed to create graphics pipeline !");

	// DESTROY SHADER MODULES
	vkDestroyShaderModule(gpu->getLogicalDevice(), fragShaderModule, nullptr);
	vkDestroyShaderModule(gpu->getLogicalDevice(), vertShaderModule, nullptr);
}

void			VkHandler::createCmdPool()
{
	uint32_t const*		queuesIndex = gpu->getQueuesIndex();

	for (size_t id = 0; id < 4; id++) {
		VkCommandPoolCreateInfo			poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queuesIndex[id];
		if (id == 3)
			poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		if (vkCreateCommandPool(gpu->getLogicalDevice(), &poolInfo, nullptr, &cmdPools[id]) != VK_SUCCESS)
			throw std::runtime_error("failed to create command pool !");
	}
}

void			VkHandler::createCmdBuffers()
{
	std::vector<VkFramebuffer> const&	framebuffers = dispHandler->getFramebuffers();
	cmdBuffers.resize(framebuffers.size());

	VkCommandBufferAllocateInfo		cmdBuffInfo = {};
	cmdBuffInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBuffInfo.commandPool = cmdPools[1];
	cmdBuffInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBuffInfo.commandBufferCount = static_cast<uint32_t>(cmdBuffers.size());
	if (vkAllocateCommandBuffers(gpu->getLogicalDevice(), &cmdBuffInfo, cmdBuffers.data()) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate command buffers");

	for (size_t i = 0; i < cmdBuffers.size(); i++) {
		VkCommandBufferBeginInfo		beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		vkBeginCommandBuffer(cmdBuffers[i], &beginInfo);

		VkRenderPassBeginInfo			rpBeginInfo = {};
		rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rpBeginInfo.renderPass = renderPass;
		rpBeginInfo.framebuffer = framebuffers[i];
		rpBeginInfo.renderArea.offset = { 0, 0 };
		rpBeginInfo.renderArea.extent = dispHandler->getScExtent();
		VkClearValue					clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		rpBeginInfo.clearValueCount = 1;
		rpBeginInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(cmdBuffers[i], &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, gfxPipeline);
		VkBuffer		vtxBuffs[] = { vertexBuffer };
		VkDeviceSize	offsets[] = { 0 };
		vkCmdBindVertexBuffers(cmdBuffers[i], 0, 1, vtxBuffs, offsets);
		vkCmdBindIndexBuffer(cmdBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT16);
		vkCmdDrawIndexed(cmdBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
		vkCmdEndRenderPass(cmdBuffers[i]);

		if (vkEndCommandBuffer(cmdBuffers[i]) != VK_SUCCESS)
			throw std::runtime_error("failed to record command buffer !");
	}
}

VkShaderModule	VkHandler::createShaderModuleFromSrc(const std::string& filename)
{
	std::ifstream	shaderFile(filename, std::ios::ate | std::ios::binary);
	if (!shaderFile.is_open())
		throw std::runtime_error("failed to open shader file !");
	size_t	filesize = static_cast<size_t>(shaderFile.tellg());
	std::vector<char>	shaderCode(filesize);
	shaderFile.seekg(0);
	shaderFile.read(shaderCode.data(), filesize);
	shaderFile.close();

	VkShaderModuleCreateInfo	shaderInfo= {};
	shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderInfo.codeSize = shaderCode.size();
	shaderInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

	VkShaderModule		shaderModule;
	if (vkCreateShaderModule(gpu->getLogicalDevice(), &shaderInfo, nullptr, &shaderModule) != VK_SUCCESS)
		throw std::runtime_error("failed to create shader module !");
	shaderCode.clear();
	return shaderModule;
}


void			VkHandler::initVulkan()
{
	VkDevice const&		gpuLDev = gpu->getLogicalDevice();

	dispHandler->createSwapchain(VK_NULL_HANDLE, gpu->getPhysicalDevice(), gpuLDev);
	dispHandler->createImgViews(gpuLDev);
	createRenderPass();
	createGFXPipeline();
	dispHandler->createFrameBuffers(gpuLDev, renderPass);
	createCmdPool();
	createVertexBuffer();
	createIndexBuffer();
	createCmdBuffers();
	createSemaphores();
}

void		VkHandler::createVertexBuffer()
{
	vertexObjectSize = static_cast<uint32_t>(vertices.size());
	transferBufferToGpuStaged((void *)vertices.data(), sizeof(vertices[0]) * vertices.size(), vertexBuffer, vertexBufferMemory, 0, 0);
}

void VkHandler::transferBufferToGpuStaged(void const* bufferData, VkDeviceSize const bufferDataSize, VkBuffer& dstBuffer,
											VkDeviceMemory& dstBufferMemory, int const copySrcOffst, int const copyDstOffst)
{
	VkBuffer			stagingBuffer;
	VkDeviceMemory		stagingBufferMem;
	VkDevice const&		gpuDev = gpu->getLogicalDevice();

	createBuffer(bufferDataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMem);

	void*		data;
	vkMapMemory(gpuDev, stagingBufferMem, 0, bufferDataSize, 0, &data);
	memcpy(data, bufferData, (size_t)bufferDataSize);
	vkUnmapMemory(gpuDev, stagingBufferMem);

	createBuffer(bufferDataSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, dstBuffer, dstBufferMemory);
	
	VkBufferCopy	copyInfo[1] = {};
	copyInfo[0].srcOffset = copySrcOffst;
	copyInfo[0].dstOffset = copyDstOffst;
	copyInfo[0].size = bufferDataSize;

	copyBuffer(stagingBuffer, dstBuffer, copyInfo, 1, VK_NULL_HANDLE);

	vkDestroyBuffer(gpuDev, stagingBuffer, nullptr);
	vkFreeMemory(gpuDev, stagingBufferMem, nullptr);
}


void VkHandler::createIndexBuffer()
{
	transferBufferToGpuStaged((void const*)indices.data(), sizeof(indices[0]) * indices.size(), indexBuffer, indexBufferMemory, 0, 0);
}

void		VkHandler::copyBuffer(VkBuffer src, VkBuffer dst, VkBufferCopy *copyInfo, uint32_t copyInfoSize, VkFence fence)
{
	VkDevice const&		gpuDev = gpu->getLogicalDevice();

	VkCommandBufferAllocateInfo		allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = cmdPools[3];
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer		cmdBuff;
	vkAllocateCommandBuffers(gpuDev, &allocInfo, &cmdBuff);

	VkCommandBufferBeginInfo		beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(cmdBuff, &beginInfo);
	vkCmdCopyBuffer(cmdBuff, src, dst, copyInfoSize, copyInfo);
	vkEndCommandBuffer(cmdBuff);

	VkSubmitInfo	submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuff;

	VkQueue const&	transferQueue = gpu->getTransferQueue();
	vkQueueSubmit(transferQueue, 1, &submitInfo, fence);
	vkQueueWaitIdle(transferQueue);

	vkFreeCommandBuffers(gpuDev, cmdPools[3], 1, &cmdBuff);
}

void		VkHandler::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
			VkMemoryPropertyFlags memProperties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkDevice const&		gpuDev = gpu->getLogicalDevice();
	uint32_t const*		queuesIndex = gpu->getQueuesIndex();

	VkBufferCreateInfo		bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;

	// Better have exlusivity then use a barrier in case of a staging buffer
	if (queuesIndex[0] != queuesIndex[3]) {
		bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		bufferInfo.pQueueFamilyIndices = queuesIndex;
	}
	else
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	//
	if (vkCreateBuffer(gpuDev, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		throw std::runtime_error("Failed to create vertex buffer");

	VkMemoryRequirements	memRequirements;
	vkGetBufferMemoryRequirements(gpuDev, buffer, &memRequirements);

	VkMemoryAllocateInfo	allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(gpu->getPhysicalDevice(), memRequirements.memoryTypeBits, memProperties);
	if (vkAllocateMemory(gpuDev, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate vertex buffer memory !");
	}
	vkBindBufferMemory(gpuDev, buffer, bufferMemory, 0);
}
uint32_t	VkHandler::findMemoryType(VkPhysicalDevice physicalGPU, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties	memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalGPU, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;
	}
	throw std::runtime_error("Failed to find a suitable memory type for buffer memory allocation !");
}

void		VkHandler::mainLoop()
{
	while (!glfwWindowShouldClose(dispHandler->getWindow())) {
		glfwPollEvents();
		drawFrame();
	}
}

void		VkHandler::createSemaphores()
{
	VkDevice const&	gpuDev = gpu->getLogicalDevice();

	VkSemaphoreCreateInfo			semInfo = {};
	semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	if (vkCreateSemaphore(gpuDev, &semInfo, nullptr, &semImgAvailable) != VK_SUCCESS ||
		vkCreateSemaphore(gpuDev, &semInfo, nullptr, &semRenderFinished) != VK_SUCCESS) {
		throw std::runtime_error("failed to create semaphores!");
	}
}

void		VkHandler::drawFrame()
{
	uint32_t				imgIndex;
	VkResult				scState;
	VkSwapchainKHR const&	swapchain = dispHandler->getSwapchain();

	scState = vkAcquireNextImageKHR(gpu->getLogicalDevice(), swapchain, std::numeric_limits<uint64_t>::max(),
									semImgAvailable, VK_NULL_HANDLE, &imgIndex);

	VkSubmitInfo	submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore				waitSem[] = { semImgAvailable };
	VkPipelineStageFlags	waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSem;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffers[imgIndex];
	VkSemaphore				sigSem[] = { semRenderFinished };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = sigSem;

	if (vkQueueSubmit(gpu->getGfxQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
		throw std::runtime_error("Failed to submit draw command buffer !");

	// PRESENTATION
	VkPresentInfoKHR		presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = sigSem;
	presentInfo.pImageIndices = &imgIndex;

	VkSwapchainKHR			scPresent[] = { swapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = scPresent;

	vkQueuePresentKHR(gpu->getPresentQueue(), &presentInfo);
	vkQueueWaitIdle(gpu->getPresentQueue());
}

void		VkHandler::resizeWindow(const int newSizeX, const int newSizeY, const bool fullscreen)
{
	dispHandler->resizeWindow(newSizeX, newSizeY, fullscreen);
	recreateSwapChain();
}

void		VkHandler::recreateSwapChain()
{
	VkDevice const&		gpuDev = gpu->getLogicalDevice();

	cleanupSwapChainAssets();
	dispHandler->createSwapchain(dispHandler->getSwapchain(), gpu->getPhysicalDevice(), gpuDev);
	dispHandler->createImgViews(gpuDev);
	createRenderPass();
	createGFXPipeline();
	dispHandler->createFrameBuffers(gpuDev, renderPass);
	createCmdBuffers();
}

void		VkHandler::cleanupSwapChainAssets()
{
	VkDevice const&		gpuDev = gpu->getLogicalDevice();

	dispHandler->destroyFramebuffers(gpuDev);
	vkFreeCommandBuffers(gpuDev, cmdPools[1], static_cast<uint32_t>(cmdBuffers.size()), cmdBuffers.data());
	vkDestroyPipeline(gpuDev, gfxPipeline, nullptr);
	vkDestroyPipelineLayout(gpuDev, pipelineLayout, nullptr);
	vkDestroyRenderPass(gpuDev, renderPass, nullptr);
	dispHandler->destroyImgViews(gpuDev);
}

void		VkHandler::terminateVulkan()
{
	VkDevice const&		gpuDev = gpu->getLogicalDevice();

	cleanupSwapChainAssets();
	dispHandler->destroySwapchain(gpuDev);
	vkDestroyBuffer(gpuDev, vertexBuffer, nullptr);
	vkFreeMemory(gpuDev, vertexBufferMemory, nullptr);
	vkDestroyBuffer(gpuDev, indexBuffer, nullptr);
	vkFreeMemory(gpuDev, indexBufferMemory, nullptr);
	if (enableValidationLayers) {
		DestroyDebugReportCallbackEXT(instance, callback, nullptr);
	}
	vkDestroySemaphore(gpuDev, semRenderFinished, nullptr);
	vkDestroySemaphore(gpuDev, semImgAvailable, nullptr);
	for (size_t i = 0; i < 4; i++) {
		vkDestroyCommandPool(gpuDev, cmdPools[i], nullptr);
	}
	dispHandler->destroySurface(instance);
	vkDestroyDevice(gpuDev, nullptr);
	vkDestroyInstance(instance, nullptr);
}