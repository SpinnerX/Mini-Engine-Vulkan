#include <Mini-GameEngine/Application.h>

#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_vulkan.h>
#include <stdio.h>
#include <stdlib.h>

/* #define GLFW_INCLUDE_NONE */
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>



/* #include "ImGui/Roboto-Regular.embed" */
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

	void check_vk_result(VkResult err){
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

	
	static void setupVulkan(const char** extension, uint32_t extensionCount){
		VkResult err;
		
		// Creating a vulkan instance.
		{
			VkInstanceCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			createInfo.enabledExtensionCount = extensionCount;
			createInfo.ppEnabledExtensionNames = extension;
		#ifdef IMGUI_VULKAN_DEBUG_REPORT
			const char* layers[] = { "VK_LAYER_KHRONOS_validation" };
			createInfo.enabledLayerCount = 1;
			createInfo.ppEnabledLayerNames = layers;

			// Enabling debugging reports extensions (we need additional storage, so duplicating user array to add new extension to it)
			const char** extensionExt = (const char**)malloc(sizeof(const char *) * (extensionCount + 1));
			memcpy(extensionExt, extension, extensionCount + sizeof(const char *));
			extensionExt[extensionCount] = "VK_EXT_debug_report";
			createInfo.enabledExtensionCount = extensionCount +1;
			createInfo.ppEnabledExtensionNames = extensionExt;

			// Creating vulkan instance
			err = vkCreateInstance(&createInfo, g_Allocator, &g_instance);
			check_vk_result(err);
			free(extensionExt);

			// Getting function ptr  (required for any extension)
			auto vkCreateDebugReportCallbackExt = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(g_instance, "vkCreateDebugReportCallbackEXT");
			IM_ASSERT(vkCreateDebugReportCallbackExt != nullptr);

			// Setting up the debug report callback
			VkDebugReportCallbackEXT debugReportCI = {};

			debugReportCI.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
			debugReportCI.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
			debugReportCI.pfnCallback = debug_report;
			debugReportCI.pUserData = NULL;
			err = vkCreateDebugReportCallbackEXT(g_instance, &debugReportCI, g_Allocator, &g_debugReport);
			check_vk_result(err);
		#else
			err = vkCreateInstance(&createInfo, g_Allocator, &g_instance);
			check_vk_result(err);
			IM_UNUSED(g_debugReport);
		#endif

			// Select GPU
			{
				uint32_t gpuCount;
				err = vkEnumeratePhysicalDevices(g_instance, &gpuCount, nullptr);
				check_vk_result(err);
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
				const char* deviceExtensions[] = { "VK_KHR_swapchain" };
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
				check_vk_result(err);
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
			check_vk_result(err);
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

		check_vk_result(err);

		_currentFrameIndex = (_currentFrameIndex + 1) % g_mainWindowData.ImageCount;

		ImGui_ImplVulkanH_Frame* frameData = &window->Frames[window->FrameIndex];

		{
			err = vkWaitForFences(g_device, 1, &frameData->Fence, VK_TRUE, UINT64_MAX);
			check_vk_result(err);

			err = vkResetFences(g_device, 1, &frameData->Fence);
			check_vk_result(err);
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
			check_vk_result(err);

			VkCommandBufferBeginInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			err = vkBeginCommandBuffer(frameData->CommandBuffer, &info);
			check_vk_result(err);
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
			check_vk_result(err);

			err = vkQueueSubmit(g_queue, 1, &info, frameData->Fence);
			check_vk_result(err);
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

		check_vk_result(err);
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
		setupVulkan(extensions, extensionCount);

		// Create Window Surface
		VkSurfaceKHR surface;
		/* VkResult err = glfwCreateWindowSurface(g_instance, _windowHandle, g_Allocator, &surface); */
		VkResult err = glfwCreateWindowSurface(g_instance, _windowHandle, g_Allocator, &surface);
		
	}


};
