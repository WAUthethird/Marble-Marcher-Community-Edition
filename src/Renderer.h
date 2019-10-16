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
	//TODO in v1.5
};

std::vector<fs::path> GetFilesInFolder(std::string folder, std::string filetype);

class Renderer
{
public:
	Renderer(int w, int h, std::string config);
	Renderer(std::string config_file);
	Renderer();

	void LoadConfigs(std::string config_file);

	void ClearTextures();

	void ClearShaders();

	void Initialize(int w, int h, std::string config = "");
	void ReInitialize(int w, int h);

	void SetOutputTexture(sf::Texture& tex);
	void LoadShader(std::string shader_file);
	std::vector<std::string> GetConfigurationsList();
	std::string GetConfigFolder();
	void LoadExternalTextures(std::string texture_folder);

	void Render();

	float EvaluateAvgIllumination();

	std::vector<ComputeShader> shader_pipeline;
	Camera camera;
	std::map<std::string, float> variables;

private:
	GLuint GenerateTexture(float w, float h);
	float illumination;
	std::string config_file;
	std::string config_folder;
	int width, height;
	
	std::vector<std::string> rendering_configurations;
	std::vector<vec2> global_size;
	std::vector<GLuint> main_textures;
	std::vector<std::vector<GLuint>> shader_textures;
	std::vector<sf::Texture> input_textures;

	GLuint illumination_texture;
	ComputeShader weight_shader;
};