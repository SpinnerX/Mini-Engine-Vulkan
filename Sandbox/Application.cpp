#include <Mini-GameEngine/Application.h>
#include <Mini-GameEngine/EntryPoint.h>
#include <Mini-GameEngine/Image.h>
#include <imgui/imgui.h>
class ExampleLayer : public MiniGameEngine::Layer
{
public:
	virtual void onUIRender() override
	{
		ImGui::Begin("Hello");
		ImGui::Button("Button");
		ImGui::End();

		ImGui::ShowDemoWindow();
	}
};

MiniGameEngine::Application* MiniGameEngine::CreateApplication(int argc, char** argv)
{
	MiniGameEngine::ApplicationSpecification spec;
	spec.name = "MiniGameEngine Example";

	MiniGameEngine::Application* app = new MiniGameEngine::Application(spec);
	app->pushLayer<ExampleLayer>();
	app->setMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}