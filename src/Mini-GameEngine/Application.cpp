#include <Mini-GameEngine/Application.h>

#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_vulkan.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <vulkan/vulkan_core.h>

/* #define GLFW_INCLUDE_NONE */
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <Mini-GameEngine/ImGui/Roboto-Regular.embed>

extern bool g_applicationRunning;

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

//#define IMGUI_UNLIMITED_FRAME_RATE
#ifdef _DEBUG
#define IMGUI_VULKAN_DEBUG_REPORT
#endif

namespace MiniGameEngine{
	
	static VkAllocationCallbacks* g_Allocator = nullptr;
	
	static VkInstance g_instance = VK_NULL_HANDLE;

	static VkPhysicalDevice g_physicalDevice = VK_NULL_HANDLE;

	static VkDevice g_device = VK_NULL_HANDLE;

	static uint32_t g_queueFamilty =  (uint32_t)-1;
	
	static VkQueue g_queue = VK_NULL_HANDLE;

	static VkDebugReportCallbackEXT g_debugReport = VK_NULL_HANDLE;

	static VkPipelineCache g_pipelineCache = VK_NULL_HANDLE;

	static VkDescriptorPool g_descriptorPool = VK_NULL_HANDLE;

	static ImGui_ImplVulkanH_Window g_mainWindowData;

	static int g_minImageCOunt = 2;

	static bool g_swapChainRebuild = false;

	// per frame in flight
	static std::vector<std::vector<VkCommandBuffer>> _allocatedCommandBuffers;
	static std::vector<std::vector<std::function<void()>>> _resourceFreeQueue;

	// Unline g_mainWindowData.frameIdx, this is not the swap chain image index.
	// and iis always guranteed to increase (e.g. 0, 1, 2, 0, 1, 2...)
	static uint32_t _currentFrameIndex = 0;

	static MiniGameEngine::Application* _instance = nullptr;

	void check_vk_result(const char* function, int line, VkResult err){
		if(err == 0)
			return;
		std::string resultString = "Default";
		
		switch (err){
        case VK_SUCCESS:
            resultString = "SUCCESS";
			break;
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            resultString = "OUT OF HOST MEMORY";
			break;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            resultString = "OUT OF DEVICE MEMORY";
			break;
        case VK_ERROR_INITIALIZATION_FAILED:
            resultString = "INITIALIZATION FAILED";
			break;
        case VK_ERROR_LAYER_NOT_PRESENT:
            resultString = "LAYER NOT PRESENT";
			break;
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            resultString = "EXTENSION NOT PRESENT";
			break;
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            resultString = "INCOMPATIBLE DRIVER";
			break;
        default:
			resultString = "Default";
            break;
		}

		printf("[vulkan] Error: VkResult = %d at function = %s on line %d [STATUS = %s]\n", err, function, line, resultString.c_str());

		if(err < 0)
			std::abort();
	}
	
	void check_vk_result2(VkResult err){
		if(err == 0)
			return;
		printf("[vulkan] Error: VkResult = %d\n", err);

		if(err < 0)
			std::abort();
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportCallbackEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* playerPrefix, const char* pMessage, void* pUserData){
		(void)flags; (void)object; (void)location; (void)messageCode; (void)pUserData; (void)playerPrefix;

		printf("[vulkan] Debug Report from ObjectType: %i\nMessage: %s\n\n", objectType, pMessage);
		return VK_FALSE;
	}

	
	// static void setupVulkan(const char** extension, uint32_t extensionCount){
	static void setupVulkan(std::vector<const char*>& extension, uint32_t extensionCount){
		VkResult err;
		
		// Creating a vulkan instance.
		{
			// "VK_KHR_surface",
			// "VK_EXT_metal_surface",

			// VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
			// VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
			// VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,

		// 	extension.push_back("VK_EXT_metal_surface");
		// 	extension.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

		// 	VkInstanceCreateInfo createInfo = {};
		// 	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		// 	// createInfo.enabledExtensionCount = extensionCount;
		// 	// createInfo.ppEnabledExtensionNames = extension;
		// 	createInfo.enabledExtensionCount = static_cast<uint32_t>(extension.size());
		// 	createInfo.ppEnabledExtensionNames = extension.data();
		// 	// createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
		// #ifdef IMGUI_VULKAN_DEBUG_REPORT
		// 	const char* layers[] = { "VK_LAYER_KHRONOS_validation" };
		// 	createInfo.enabledLayerCount = 1;
		// 	createInfo.ppEnabledLayerNames = layers;

		// 	// Enabling debugging reports extensions (we need additional storage, so duplicating user array to add new extension to it)
		// 	const char** extensionExt = (const char**)malloc(sizeof(const char *) * (extensionCount + 1));
		// 	memcpy(extensionExt, extension, extensionCount + sizeof(const char *));
		// 	extensionExt[extensionCount] = "VK_EXT_debug_report";
		// 	createInfo.enabledExtensionCount = extensionCount +1;
		// 	createInfo.ppEnabledExtensionNames = extensionExt;

		// 	// Creating vulkan instance
		// 	err = vkCreateInstance(&createInfo, g_Allocator, &g_instance);
		// 	check_vk_result(err);
		// 	free(extensionExt);

		// 	// Getting function ptr  (required for any extension)
		// 	auto vkCreateDebugReportCallbackExt = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(g_instance, "vkCreateDebugReportCallbackEXT");
		// 	IM_ASSERT(vkCreateDebugReportCallbackExt != nullptr);

		// 	// Setting up the debug report callback
		// 	VkDebugReportCallbackEXT debugReportCI = {};

		// 	debugReportCI.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		// 	debugReportCI.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
		// 	debugReportCI.pfnCallback = debug_report;
		// 	debugReportCI.pUserData = NULL;
		// 	err = vkCreateDebugReportCallbackEXT(g_instance, &debugReportCI, g_Allocator, &g_debugReport);
		// 	check_vk_result(err);
		// #else
		// 	err = vkCreateInstance(&createInfo, g_Allocator, &g_instance);
		// 	check_vk_result(__FUNCTION__, __LINE__, err);
		// 	IM_UNUSED(g_debugReport);
		// #endif
		VkResult err;
	#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
		volkInitialize();
	#endif

		// Create Vulkan Instance
		{
				VkInstanceCreateInfo create_info = {};
				create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

				// Enumerate available extensions
				uint32_t properties_count;
				ImVector<VkExtensionProperties> properties;
				vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, nullptr);
				properties.resize(properties_count);
				err = vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, properties.Data);
				// check_vk_result(err);

				// Enable required extensions
				// if (IsExtensionAvailable(properties, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
				extension.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
		#ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
				// if (IsExtensionAvailable(properties, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)){
				extension.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
				create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
				// }
		#endif

				// Enabling validation layers
		#ifdef APP_USE_VULKAN_DEBUG_REPORT
				const char* layers[] = { "VK_LAYER_KHRONOS_validation" };
				create_info.enabledLayerCount = 1;
				create_info.ppEnabledLayerNames = layers;
				extension.push_back("VK_EXT_debug_report");
		#endif

				// Create Vulkan Instance
				create_info.enabledExtensionCount = (uint32_t)extension.size();
				create_info.ppEnabledExtensionNames = extension.data();
				err = vkCreateInstance(&create_info, g_Allocator, &g_instance);
				// check_vk_result(err);
				check_vk_result(__FUNCTION__, __LINE__, err);
			}

			// Select GPU
			{
				uint32_t gpuCount;
				err = vkEnumeratePhysicalDevices(g_instance, &gpuCount, nullptr);
				check_vk_result(__FUNCTION__, __LINE__, err);
				IM_ASSERT(gpuCount > 1);

				VkPhysicalDevice* gpus = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * gpuCount);
				err = vkEnumeratePhysicalDevices(g_instance, &gpuCount, gpus);

				// @note if number > 1 of GPU's got reported, find discrete GPU present or use first one available.
				// @note Covers most common cases (multi-gpu/integrated+dedicated gpu's)
				// @note handling more complicated setups (multiple gpu's) out of scope of this sample.
				int usedGPU = 0;

				for(int i = 0; i < (int)gpuCount; i++){
					VkPhysicalDeviceProperties properties;
					vkGetPhysicalDeviceProperties(gpus[i], &properties);

					if(properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
						usedGPU = i;
						break;
					}
				}

				g_physicalDevice = gpus[usedGPU];
				free(gpus);
			}

			// Selecting graphics queue familty
			{
				uint32_t count;
				vkGetPhysicalDeviceQueueFamilyProperties(g_physicalDevice, &count, nullptr);
				VkQueueFamilyProperties* queues = (VkQueueFamilyProperties *)malloc(sizeof(VkQueueFamilyProperties) * count);
				vkGetPhysicalDeviceQueueFamilyProperties(g_physicalDevice, &count, queues);

				for(uint32_t i = 0; i < count; i++){
					if(queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT){
						g_queueFamilty = i;
						break;
					}
				}

				free(queues);
				IM_ASSERT(g_queueFamilty != (uint32_t)-1);
			}

			// Create Logical Device (with 1 queued)
			{
				int deviceExtensionCount = 1;
				const char* deviceExtensions[] = { "VK_KHR_swapchain", "VK_EXT_metal_surface", };
				const float queuePriority[] = { 1.0f };
				VkDeviceQueueCreateInfo queueInfo[1] = {};
				queueInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo[0].queueFamilyIndex = g_queueFamilty;
				queueInfo[0].queueCount = 1;
				queueInfo[0].pQueuePriorities = queuePriority;

				VkDeviceCreateInfo createInfo = {};
				createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
				createInfo.queueCreateInfoCount = sizeof(queueInfo) / sizeof(queueInfo[0]);
				createInfo.pQueueCreateInfos = queueInfo;
				createInfo.enabledExtensionCount = deviceExtensionCount;
				createInfo.ppEnabledExtensionNames = deviceExtensions;

				err = vkCreateDevice(g_physicalDevice, &createInfo, g_Allocator, &g_device);
				check_vk_result(__FUNCTION__, __LINE__, err);
				vkGetDeviceQueue(g_device, g_queueFamilty, 0, &g_queue);
			}

			// Create Descriptor Pool
			VkDescriptorPoolSize poolSizes[] = {
				{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
			};
			
			VkDescriptorPoolCreateInfo poolInfo  = {};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			poolInfo.maxSets = 1000 * IM_ARRAYSIZE(poolSizes);
			poolInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(poolSizes);
			poolInfo.pPoolSizes = poolSizes;
			err = vkCreateDescriptorPool(g_device, &poolInfo, g_Allocator, &g_descriptorPool);
			check_vk_result(__FUNCTION__, __LINE__, err);
		}
	}

	// @note All ImGui_ImplVulkan structures/functions are optional helpers used by demo
	// @note Actual engine/app may not utilze them.
	static void setupVulkanWindow(ImGui_ImplVulkanH_Window* window, VkSurfaceKHR surface, int width, int height){
		window->Surface = surface;
		
		// Check for WSI support
		VkBool32 res;
		vkGetPhysicalDeviceSurfaceSupportKHR(g_physicalDevice, g_queueFamilty, window->Surface, &res);

		if(res != VK_TRUE){
			printf("Error, no WSI support on the physica device 0\n");
			std::exit(-1);
		}

		// Selecting surface format
		const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
		const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		window->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(g_physicalDevice, window->Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

		// Select Preset Mode
	#ifdef IMGUI_UNLIMITED_FRAME_RATE
		VkPresentModeKHR presentModes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
	#else
		VkPresentModeKHR presentModes[] = { VK_PRESENT_MODE_FIFO_KHR };
	#endif
		window->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(g_physicalDevice, window->Surface, &presentModes[0], IM_ARRAYSIZE(presentModes));
		
		// @note Creating SwapChain, RenderPass, Framebuffer, etc.
		IM_ASSERT(g_minImageCOunt >= 2);
		ImGui_ImplVulkanH_CreateOrResizeWindow(g_instance, g_physicalDevice, g_device, window, g_queueFamilty, g_Allocator, width, height, g_minImageCOunt);
	}

	static void cleanupVulkan(){
		vkDestroyDescriptorPool(g_device, g_descriptorPool, g_Allocator);

	#ifdef IMGUI_VULKAN_DEBUG_REPORT
		// Remove debug report callback
		auto vkDestroyDebugReportCallbackExt = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(g_instance, "vkDestroyDebugReportCallbackEXT");
		vkDestroyDebugReportCallbackExt(g_instance, g_debugReport, g_Allocator);
	#endif

		vkDestroyDevice(g_device, g_Allocator);
		vkDestroyInstance(g_instance, g_Allocator);
	}

	static void cleanupVulkanWindow(){
		ImGui_ImplVulkanH_DestroyWindow(g_instance, g_device, &g_mainWindowData, g_Allocator);
	}

	static void frameRender(ImGui_ImplVulkanH_Window* window, ImDrawData* drawData){
		VkResult err;

		VkSemaphore imageAcquredSemaphore = window->FrameSemaphores[window->SemaphoreIndex].ImageAcquiredSemaphore;
		VkSemaphore renderCompleteSemaphore = window->FrameSemaphores[window->SemaphoreIndex].RenderCompleteSemaphore;

		if(err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR){
			g_swapChainRebuild = true;
			return;
		}

		check_vk_result(__FUNCTION__, __LINE__, err);

		_currentFrameIndex = (_currentFrameIndex + 1) % g_mainWindowData.ImageCount;

		ImGui_ImplVulkanH_Frame* frameData = &window->Frames[window->FrameIndex];

		{
			err = vkWaitForFences(g_device, 1, &frameData->Fence, VK_TRUE, UINT64_MAX);
			check_vk_result(__FUNCTION__, __LINE__, err);

			err = vkResetFences(g_device, 1, &frameData->Fence);
			check_vk_result(__FUNCTION__, __LINE__, err);
		}
		
		// Free resources in queue
		{
			for(auto& func : _resourceFreeQueue[_currentFrameIndex])
				func();

			_resourceFreeQueue[_currentFrameIndex].clear();
		}
		
		// @note Free Command Buffers allocatoed by Application::GetCommandBuffer()
		// @note These use g_mainWindowData.FrameIndex and not _currentFrameIndex because they are tied to swapchain img index.
		{
			auto& allocatedCommandBuffers = _allocatedCommandBuffers[window->FrameIndex];

			if(allocatedCommandBuffers.size() > 0){
				vkFreeCommandBuffers(g_device, frameData->CommandPool, (uint32_t)allocatedCommandBuffers.size(), allocatedCommandBuffers.data());
				allocatedCommandBuffers.clear();
			}

			err = vkResetCommandPool(g_device, frameData->CommandPool, 0);
			check_vk_result(__FUNCTION__, __LINE__, err);

			VkCommandBufferBeginInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			err = vkBeginCommandBuffer(frameData->CommandBuffer, &info);
			check_vk_result(__FUNCTION__, __LINE__, err);
		}

		{
			VkRenderPassBeginInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			info.renderPass = window->RenderPass;
			info.framebuffer = frameData->Framebuffer;
			info.renderArea.extent.width = window->Width;
			info.renderArea.extent.height = window->Height;
			info.clearValueCount = 1;
			info.pClearValues = &window->ClearValue;
			vkCmdBeginRenderPass(frameData->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
		}

		// Records imgui primitives into command buffer
		ImGui_ImplVulkan_RenderDrawData(drawData, frameData->CommandBuffer);

		// Submit Command Buffers
		vkCmdEndRenderPass(frameData->CommandBuffer);

		{
			VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			VkSubmitInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			info.waitSemaphoreCount = 1;
			info.pWaitSemaphores = &imageAcquredSemaphore;
			info.pWaitDstStageMask = &waitStage;
			info.commandBufferCount = 1;
			info.pCommandBuffers = &frameData->CommandBuffer;
			info.signalSemaphoreCount = 1;
			info.pSignalSemaphores = &renderCompleteSemaphore;

			err = vkEndCommandBuffer(frameData->CommandBuffer);
			check_vk_result(__FUNCTION__, __LINE__, err);

			err = vkQueueSubmit(g_queue, 1, &info, frameData->Fence);
			check_vk_result(__FUNCTION__, __LINE__, err);
		}
	}

	static void framePresent(ImGui_ImplVulkanH_Window* window){
		if(g_swapChainRebuild)
			return;

		VkSemaphore renderCompleteSemaphore =  window->FrameSemaphores[window->SemaphoreIndex].RenderCompleteSemaphore;
		VkPresentInfoKHR info = {};
		info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		info.waitSemaphoreCount = 1;
		info.pWaitSemaphores = &renderCompleteSemaphore;
		info.swapchainCount = 1;
		info.pSwapchains = &window->Swapchain;
		info.pImageIndices = &window->FrameIndex;

		VkResult err = vkQueuePresentKHR(g_queue, &info);

		if(err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR){
			g_swapChainRebuild = true;
			return;
		}

		check_vk_result(__FUNCTION__, __LINE__, err);
		window->SemaphoreIndex = (window->SemaphoreIndex + 1) % window->ImageCount; // @note can use next set of semaphores
	}

	static void glfw_error_callback(int error, const char* description){
		printf("GLFW Error %d: %s\n", error, description);
	}

	Application::Application(const ApplicationSpecification& appSpec) : spec(appSpec) {
		_instance = this;

		init();
	}

	Application::~Application(){
		shutdown();
		_instance = nullptr;
	}

	Application& Application::get(){
		return *_instance;
	}

	void Application::init(){
		glfwSetErrorCallback(glfw_error_callback);

		if(!glfwInit()){
			printf("Could not initialized GLFW!\n");
			return;
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		_windowHandle = glfwCreateWindow(spec.width, spec.height, spec.name.c_str(), nullptr, nullptr);

		// Setting  up vulkan
		if(!glfwVulkanSupported()){
			printf("GLFW: Vulkan not supported!\n");
			return;
		}

		uint32_t extensionCount =  0;
		const char** extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
		std::vector<const char *> extension;

		for(uint32_t i = 0; i < extensionCount; i++){
			extension.push_back(extensions[i]);
		}

		setupVulkan(extension, extensionCount);

		// Create Window Surface
		VkSurfaceKHR surface;
		VkResult err = glfwCreateWindowSurface(g_instance, _windowHandle, g_Allocator, &surface);
		
		check_vk_result(__FUNCTION__, __LINE__, err);

		// Creating Framebuffers
		int width, height;
		glfwGetFramebufferSize(_windowHandle, &width, &height);

		ImGui_ImplVulkanH_Window* wd = &g_mainWindowData;
		setupVulkanWindow(wd, surface, width, height);

		_allocatedCommandBuffers.resize(wd->ImageCount);
		_resourceFreeQueue.resize(wd->ImageCount);

		// Setting up imgui
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

		ImGui::StyleColorsDark();

		ImGuiStyle& style = ImGui::GetStyle();
		if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable){
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		// Setup Platform/Renderer backends
		// ImGui_ImplGlfw_InitForVulkan(_windowHandle, true);
		// ImGui_ImplVulkan_InitInfo init_info = {};
		// init_info.Instance = g_instance;
		// init_info.PhysicalDevice = g_physicalDevice;
		// init_info.Device = g_device;
		// init_info.QueueFamily = g_queueFamilty;
		// init_info.Queue = g_queue;
		// init_info.PipelineCache = g_pipelineCache;
		// init_info.DescriptorPool = g_descriptorPool;
		// init_info.Subpass = 0;
		// init_info.RenderPass = wd->RenderPass;
		// init_info.MinImageCount = g_minImageCOunt;
		// init_info.ImageCount = wd->ImageCount;
		// init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		// init_info.Allocator = g_Allocator;
		// init_info.CheckVkResultFn = check_vk_result2;
		// ImGui_ImplVulkan_Init(&init_info);
		// ImGui_ImplVulkan_Init(&init_info, wd->RenderPass);

		ImGui_ImplGlfw_InitForVulkan(_windowHandle, true);
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = g_instance;
		init_info.PhysicalDevice = g_physicalDevice;
		init_info.Device = g_device;
		init_info.QueueFamily = g_queueFamilty;
		init_info.Queue = g_queue;
		init_info.PipelineCache = g_pipelineCache;
		init_info.DescriptorPool = g_descriptorPool;
		init_info.RenderPass = wd->RenderPass;
		init_info.Subpass = 0;
		init_info.MinImageCount = g_minImageCOunt;
		init_info.ImageCount = wd->ImageCount;
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.Allocator = g_Allocator;
		init_info.CheckVkResultFn = check_vk_result2;
		ImGui_ImplVulkan_Init(&init_info);

		// Load default font
		// ImFontConfig fontConfig;
		// fontConfig.FontDataOwnedByAtlas = false;
		// ImFont* robotoFont = io.Fonts->AddFontFromMemoryTTF((void*)g_RobotoRegular, sizeof(g_RobotoRegular), 20.0f, &fontConfig);
		// io.FontDefault = robotoFont;

		// Upload Fonts
		// {
		// 	VkCommandPool commandPool = wd->Frames[wd->FrameIndex].CommandPool;
		// 	VkCommandBuffer commandBuffer = wd->Frames[wd->FrameIndex].CommandBuffer;

		// 	err = vkResetCommandPool(g_device, commandPool, 0);
		// 	check_vk_result(__FUNCTION__, __LINE__, err);

		// 	VkCommandBufferBeginInfo beginInfo = {};
		// 	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		// 	beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		// 	err = vkBeginCommandBuffer(commandBuffer, &beginInfo);
		// 	check_vk_result(__FUNCTION__, __LINE__, err);

		// 	/* ImGui_ImplVulkan_CreateFontsTexture(commandBuffer); */
		// 	ImGui_ImplVulkan_CreateFontsTexture();

		// 	VkSubmitInfo end_info = {};
		// 	end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		// 	end_info.commandBufferCount = 1;
		// 	end_info.pCommandBuffers = &commandBuffer;
		// 	err = vkEndCommandBuffer(commandBuffer);
		// 	check_vk_result(__FUNCTION__, __LINE__, err);
		// 	err = vkQueueSubmit(g_queue, 1, &end_info, VK_NULL_HANDLE);
		// 	check_vk_result(__FUNCTION__, __LINE__, err);

		// 	err = vkDeviceWaitIdle(g_device);
		// 	check_vk_result(__FUNCTION__, __LINE__, err);
		// 	/* ImGui_ImplVulkan_DestroyFontUploadObjects(); */
		// 	ImGui_ImplVulkan_DestroyFontsTexture();
		// }
	}
	
	void Application::shutdown(){
		for(auto& layer : _layerStack)
			layer->onDetach();

		_layerStack.clear();

		// Cleanup
		VkResult err = vkDeviceWaitIdle(g_device);
		check_vk_result(__FUNCTION__, __LINE__, err);

		// Free resources in queue
		for(auto& queue : _resourceFreeQueue){
			for(auto& func : queue)
				func();
		}

		_resourceFreeQueue.clear();

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		cleanupVulkanWindow();
		cleanupVulkan();

		glfwDestroyWindow(_windowHandle);
		glfwTerminate();

		g_applicationRunning = false;

	}

	void Application::run(){
		_isRunning = true;

		ImGui_ImplVulkanH_Window* wd = &g_mainWindowData;
		ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
		ImGuiIO& io = ImGui::GetIO();

		while(!glfwWindowShouldClose(_windowHandle)){
			// Pool and handling events (inputs, window resize, etc.)
			// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
			// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
			// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
			// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.

			glfwPollEvents();

			for(auto& layer : _layerStack)
				layer->onUpdate(_timeStep);
			
			// resize swap chain
			if(g_swapChainRebuild){
				int width, height;
				glfwGetFramebufferSize(_windowHandle, &width, &height);
				if (width > 0 && height > 0){
					ImGui_ImplVulkan_SetMinImageCount(g_minImageCOunt);
					ImGui_ImplVulkanH_CreateOrResizeWindow(g_instance, g_physicalDevice, g_device, &g_mainWindowData, g_queueFamilty, g_Allocator, width, height, g_minImageCOunt);
					g_mainWindowData.FrameIndex = 0;

					// Clear allocated command buffers from here since entire pool is destroyed
					_allocatedCommandBuffers.clear();
					_allocatedCommandBuffers.resize(g_mainWindowData.ImageCount);

					g_swapChainRebuild = false;
				}
			}

			// Starting imgui frame
			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			{
				static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;

				// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
				// because it would be confusing to have two docking targets within each others.
				ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
				if (_menubarCallback)
					window_flags |= ImGuiWindowFlags_MenuBar;

				const ImGuiViewport* viewport = ImGui::GetMainViewport();
				ImGui::SetNextWindowPos(viewport->WorkPos);
				ImGui::SetNextWindowSize(viewport->WorkSize);
				ImGui::SetNextWindowViewport(viewport->ID);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
				window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
				window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

				// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
				// and handle the pass-thru hole, so we ask Begin() to not render a background.
				if (dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
					window_flags |= ImGuiWindowFlags_NoBackground;

				// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
				// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
				// all active windows docked into it will lose their parent and become undocked.
				// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
				// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
				ImGui::Begin("DockSpace Demo", nullptr, window_flags);
				ImGui::PopStyleVar();

				ImGui::PopStyleVar(2);

				// Submit the DockSpace
				ImGuiIO& io = ImGui::GetIO();
				if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable){
					ImGuiID dockspace_id = ImGui::GetID("VulkanAppDockspace");
					ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspaceFlags);
				}

				if (_menubarCallback){
					if (ImGui::BeginMenuBar()){
						_menubarCallback();
						ImGui::EndMenuBar();
					}
				}

				for (auto& layer : _layerStack)
					layer->onUIRender();

				ImGui::End();
			}

			// Rendering
			ImGui::Render();
			ImDrawData* main_draw_data = ImGui::GetDrawData();
			const bool main_is_minimized = (main_draw_data->DisplaySize.x <= 0.0f || main_draw_data->DisplaySize.y <= 0.0f);
			wd->ClearValue.color.float32[0] = clearColor.x * clearColor.w;
			wd->ClearValue.color.float32[1] = clearColor.y * clearColor.w;
			wd->ClearValue.color.float32[2] = clearColor.z * clearColor.w;
			wd->ClearValue.color.float32[3] = clearColor.w;
			if (!main_is_minimized)
				frameRender(wd, main_draw_data);

			// Update and Render additional Platform Windows
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
			}

			// Present Main Platform Window
			if (!main_is_minimized)
				framePresent(wd);

			float time = getTime();
			_frameTime = time - _lastFrameTime;
			_timeStep = glm::min<float>(_frameTime, 0.0333f);
			_lastFrameTime = time;
		}
	}

	void Application::close(){
		_isRunning = false;
	}

	float Application::getTime(){
		return (float)glfwGetTime();
	}

	VkInstance Application::getInstance(){
		return g_instance;
	}

	VkPhysicalDevice Application::getPhysicalDevice(){
		return g_physicalDevice;
	}

	VkDevice Application::getDevice(){
		return g_device;
	}

	VkCommandBuffer Application::getCommandBuffer(bool begin){
		ImGui_ImplVulkanH_Window* wd = &g_mainWindowData;

		// Use any command queue
		VkCommandPool command_pool = wd->Frames[wd->FrameIndex].CommandPool;

		VkCommandBufferAllocateInfo cmdBufAllocateInfo = {};
		cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufAllocateInfo.commandPool = command_pool;
		cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBufAllocateInfo.commandBufferCount = 1;

		VkCommandBuffer& command_buffer = _allocatedCommandBuffers[wd->FrameIndex].emplace_back();
		auto err = vkAllocateCommandBuffers(g_device, &cmdBufAllocateInfo, &command_buffer);

		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		err = vkBeginCommandBuffer(command_buffer, &begin_info);
		check_vk_result(__FUNCTION__, __LINE__, err);

		return command_buffer;
	}

	void Application::flushCommandBuffer(VkCommandBuffer commandBuffer){
		const uint64_t DEFAULT_FENCE_TIMEOUT = 100000000000;

		VkSubmitInfo end_info = {};
		end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		end_info.commandBufferCount = 1;
		end_info.pCommandBuffers = &commandBuffer;
		auto err = vkEndCommandBuffer(commandBuffer);
		check_vk_result(__FUNCTION__, __LINE__, err);

		// Create fence to ensure that the command buffer has finished executing
		VkFenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = 0;
		VkFence fence;
		err = vkCreateFence(g_device, &fenceCreateInfo, nullptr, &fence);
		check_vk_result(__FUNCTION__, __LINE__, err);

		err = vkQueueSubmit(g_queue, 1, &end_info, fence);
		check_vk_result(__FUNCTION__, __LINE__, err);

		err = vkWaitForFences(g_device, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT);
		check_vk_result(__FUNCTION__, __LINE__, err);

		vkDestroyFence(g_device, fence, nullptr);
	}

	void Application::submitResourceFree(std::function<void ()> &&func){
		_resourceFreeQueue[_currentFrameIndex].emplace_back(func);
	}

};
