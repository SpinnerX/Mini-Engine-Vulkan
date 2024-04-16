#include <Mini-GameEngine/Random.h>

namespace MiniGameEngine {

	std::mt19937 Random::_randomEngine;
	std::uniform_int_distribution<std::mt19937::result_type> Random::_distrbution;

}