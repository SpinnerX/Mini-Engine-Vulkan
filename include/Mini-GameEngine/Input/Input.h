#pragma once

#include <Mini-GameEngine/Input/KeyCodes.h>
#include <glm/glm.hpp>

namespace MiniGameEngine{
	class Input{
	public:
		static bool isKeyPressed(KeyCode keycode);
		static bool isMouseButtonPressed(MouseButton button);

		static glm::vec2 getMousePosition();

		static void setCursorMode(CursorMode mode);
	};
};
