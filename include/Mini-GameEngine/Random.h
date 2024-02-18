#pragma once
#include <random>
#include <glm/glm.hpp>

namespace MiniGameEngine{
	class Random{
	public:
		static void init(){
			_randomEngine.seed(std::random_device()());
		}

		static uint32_t uInt(){
			return _distrbution(_randomEngine);
		}

		static uint32_t uInt(uint32_t min, uint32_t max){
			return min + (_distrbution(_randomEngine) % (max - min + 1));
		}

		static float Float(){
			return (float)_distrbution(_randomEngine) / (float)std::numeric_limits<uint32_t>::max();
		}
		
		static glm::vec3 Vec3(){
			return glm::vec3(Float(), Float(), Float());
		}

		static glm::vec3 Vec3(float min, float max){
			return glm::vec3(Float() * (max - min) + min, Float() * (max - min) + min, Float() * (max - min) + min);
		}

		static glm::vec3 inUnitSphere(){
			return glm::normalize(Vec3(-1.0f, 1.0f));
		}

	private:
		static std::mt19937 _randomEngine;
		static std::uniform_int_distribution<std::mt19937::result_type> _distrbution;
	};
};
