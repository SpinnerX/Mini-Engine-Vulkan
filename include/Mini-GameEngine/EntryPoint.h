#pragma once

extern MiniGameEngine::Application* MiniGameEngine::CreateApplication(int argc, char **argv);
bool g_applicationRunning = true;

namespace MiniGameEngine{
	int Main(int argc, char** argv){
		while(g_applicationRunning){
			MiniGameEngine::Application* app = MiniGameEngine::CreateApplication(argc, argv);
			app->run();
			delete app;
		}

		return 0;
	}
};
#ifdef MINI_GAMEENGINE_PLATFORMS
#else

// #if WL_DIST
// #else
// #endif
int main(int argc, char** argv){
	return MiniGameEngine::Main(argc, argv);
}

#endif
