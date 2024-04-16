#pragma once

#include "Layer.h"

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <imgui/imgui.h>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

namespace MiniGameEngine{

	struct ApplicationSpecification{
		std::string name = "Mini Engine3D";
		uint32_t width = 1600;
		uint32_t height = 900;
	};

	class Application{
	public:
		Application(const ApplicationSpecification& appSpec);

		~Application();

		static Application& get();

		void run();

		void setMenubarCallback(const std::function<void()>& menubarCallback) { _menubarCallback = menubarCallback; }

		void pushLayer(const std::shared_ptr<Layer>& layer){
			_layerStack.emplace_back(layer);
			layer->onAttach();
		}

		template<typename T>
		void pushLayer(){
			static_assert(std::is_base_of<Layer, T>::value, "Pushed type not subclass of Layer");
			// _layerStack.emplace_back(std::make_shared<T>())->onAttach();
			_layerStack.emplace_back(std::make_shared<T>())->onAttach();
		}

		void close();

		float getTime();

		GLFWwindow* getNativeWindow() const { return _windowHandle; }

		static VkInstance getInstance();

		static VkPhysicalDevice getPhysicalDevice();
		static VkDevice getDevice();

		static VkCommandBuffer getCommandBuffer(bool begin);

		static void flushCommandBuffer(VkCommandBuffer commandBuffer);

		static void submitResourceFree(std::function<void()>&& func);


	private:

		void init();
		void shutdown();

	private:
		ApplicationSpecification spec;
		GLFWwindow* _windowHandle = nullptr;
		float _isRunning=false;
		float _timeStep = 0.0f;
		float _frameTime = 0.0f;
		float _lastFrameTime = 0.0f;

		std::vector<std::shared_ptr<Layer>> _layerStack;
		std::function<void()> _menubarCallback;
	};
	
	// Implemented by client.
	Application* CreateApplication(int argc, char** argv);
};
