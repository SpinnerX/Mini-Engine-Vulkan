#include <Mini-GameEngine/Input/Input.h>
#include <Mini-GameEngine/Application.h>
#include <GLFW/glfw3.h>

namespace MiniGameEngine{
	bool Input::isKeyPressed(KeyCode keycode){
		GLFWwindow* windowHandle = Application::get().getNativeWindow();
		int state = glfwGetKey(windowHandle, (int)keycode);

		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool Input::isMouseButtonPressed(MouseButton button){
		GLFWwindow* windowHandle = Application::get().getNativeWindow();
		int state = glfwGetMouseButton(windowHandle, (int)button);
		return state == GLFW_PRESS;
	}

	glm::vec2 Input::getMousePosition(){
		GLFWwindow* windowHandle = Application::get().getNativeWindow();
		
		double x, y;
		glfwGetCursorPos(windowHandle, &x, &y);
		
		return {x, y};
	}

	void Input::setCursorMode(CursorMode mode){
		GLFWwindow* windowHandle = Application::get().getNativeWindow();
		glfwSetInputMode(windowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL + (int)mode);
	}
};
