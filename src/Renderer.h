#pragma once

#include<Shaders.h>
#include<ExprParser.h>

//TODO
class Wobject
{
	vec3 position;
	vec3 radius; //bounding volume radius

};

class WorldModel
{

};

std::vector<fs::path> GetFilesInFolder(std::string folder, std::string filetype);

class Renderer
{
public:
	Renderer(int w, int h, std::string config);
	Renderer();

	void Initialize(int w, int h, std::string config);
	void ReInitialize(int w, int h);

	void SetOutputTexture(sf::Texture& tex);
	void LoadShader(std::string shader_file);
	std::vector<std::string> GetConfigurationsList();
	std::string GetConfigFolder();

	void Render();
	std::vector<ComputeShader> shader_pipeline;
	Camera camera;

private:
	std::string config_file;
	std::string config_folder;
	GLuint GenerateTexture(float w, float h);

	int width, height;

	std::map<std::string, float> variables;
	std::vector<std::string> rendering_configurations;
	std::vector<vec2> global_size;
	std::vector<GLuint> main_textures;
	std::vector<std::vector<GLuint>> shader_textures;
};