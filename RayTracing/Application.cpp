#include "Mini-GameEngine/Application.h"
#include "Mini-GameEngine/EntryPoint.h"
#include "ExampleLayer.h"
#include <imgui/imgui.h>

MiniGameEngine::Application* MiniGameEngine::CreateApplication(int argc, char** argv){
	MiniGameEngine::ApplicationSpecification spec;
	spec.name = "Ray Tracing";

	MiniGameEngine::Application* app = new MiniGameEngine::Application(spec);
	app->pushLayer<ExampleLayer>();
	app->setMenubarCallback([app](){
		if (ImGui::BeginMenu("File")){
			if (ImGui::MenuItem("Exit")){
				app->close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}