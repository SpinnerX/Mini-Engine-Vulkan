#pragma once

#include <glm/glm.hpp>

#include <vector>

/**
 * @name Albedo
 * @note Gives color of diffuse lighting reflected from the surface when lit with incident light of unit radiance normaled to surface
 * 
 * 
 * 
*/

struct Sphere{
	glm::vec3 Position{0.0f};
	float Radius = 0.5f;

	glm::vec3 Albedo{1.0f};
};

struct Cube{
	glm::vec3 Position{0.f};
	uint32_t width=0, height=0;
	glm::vec3 Albedo{1.f};
};

struct WorldPlatform{
	glm::vec3 Position{0.f};
	uint32_t width = 0, height = 0;
};

struct Scene{

	//! @note Our spheres will be here
	//! @note Should probably have this be a map based on our UUID
	//! @idea map<UUID, Object>
	std::vector<Sphere> Spheres;
};