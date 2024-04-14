#include "ExampleLayer.h"
#include <imgui/imgui.h>
using namespace MiniGameEngine;

ExampleLayer::ExampleLayer()
	: camera(45.0f, 0.1f, 100.0f){
	{
		Sphere sphere;
		sphere.Position = { 0.0f, 0.0f, 0.0f };
		sphere.Radius = 0.5f;
		sphere.Albedo = { 1.0f, 0.0f, 1.0f };
		scene.Spheres.push_back(sphere);
	}

	{
		Sphere sphere;
		sphere.Position = { 1.0f, 0.0f, -5.0f };
		sphere.Radius = 1.5f;
		sphere.Albedo = { 0.2f, 0.3f, 1.0f };
		scene.Spheres.push_back(sphere);
	}
}

void ExampleLayer::onUpdate(float ts){
	camera.onUpdate(ts);
}

void ExampleLayer::onUIRender(){
	ImGui::Begin("Settings");
	ImGui::Text("Last render: %.3fms", lastRenderTime);
	if (ImGui::Button("Render")){
		Render();
	}
	ImGui::End();

	ImGui::Begin("Scene");
	for (size_t i = 0; i < scene.Spheres.size(); i++){
		ImGui::PushID(i);

		Sphere& sphere = scene.Spheres[i];
		ImGui::DragFloat3("Position", glm::value_ptr(sphere.Position), 0.1f);
		ImGui::DragFloat("Radius", &sphere.Radius, 0.1f);
		ImGui::ColorEdit3("Albedo", glm::value_ptr(sphere.Albedo));

		ImGui::Separator();

		ImGui::PopID();
	}
	ImGui::End();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("Viewport");

	viewportWidth = ImGui::GetContentRegionAvail().x;
	viewportHeight = ImGui::GetContentRegionAvail().y;

	auto image = renderer.getFinalImage();
	if (image)
		ImGui::Image(image->getDescriptorSet(), { (float)image->getWidth(), (float)image->getHeight() },
			ImVec2(0, 1), ImVec2(1, 0));

	ImGui::End();
	ImGui::PopStyleVar();

	Render();
}

void ExampleLayer::Render(){
	
	Timer timer;

	Sphere& sphere1 = scene.Spheres[0];
	sphere1.Position.y += 1;


	renderer.onResize(viewportWidth, viewportHeight);
	camera.onResize(viewportWidth, viewportHeight);
	renderer.Render(scene, camera);

	lastRenderTime = timer.elapsedMillis();
}