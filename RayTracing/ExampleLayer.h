#pragma once
#include <Mini-GameEngine/Layer.h>
#include "Mini-GameEngine/Timer.h"

#include "Renderer.h"
#include "Camera.h"

#include <glm/gtc/type_ptr.hpp>

// using namespace MiniGameEngine;

class ExampleLayer : public MiniGameEngine::Layer{
public:
	ExampleLayer();

	virtual void onUpdate(float ts) override;

	virtual void onUIRender() override;

	void Render();

private:
	Renderer renderer;
	Camera camera;
	Scene scene;
	uint32_t viewportWidth = 0, viewportHeight = 0;

	float lastRenderTime = 0.0f;
};