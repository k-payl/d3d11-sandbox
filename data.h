#pragma once
#include "library.h"

struct Data
{
	Mesh *mesh;
	std::unique_ptr<Texture> albedoTexture;
	std::unique_ptr<Texture> normalTexture;
	std::unique_ptr<Texture> heighTexture;
	std::unique_ptr<Texture> metallicTexture;
	std::unique_ptr<Texture> roomEnvironment;
	std::unique_ptr<Texture> angarEnvironment;
	std::unique_ptr<Texture> nightEnvironment;
	std::unique_ptr<Camera> camera;
	std::unique_ptr<Shader> forward;
	std::unique_ptr<Mesh> cubeMesh;
	std::unique_ptr<Mesh> planeMesh;
	std::unique_ptr<Mesh> teapotMesh;
	std::string CompileError;
	int meshIndex{0};
	int cubemapIndex{0};
	bool rotate{true};
	float scale{ 1 };
	float rotationSpeed{.25f};
	bool VSync{true};
};