#include "Renderer.h"
#include "Renderer.h"
#include "Renderer.h"


Renderer::Renderer(int w, int h, std::string config_file)
{
	LoadConfigs(config_file);
	Initialize(w, h, config_file);
}

Renderer::Renderer(std::string config_file)
{
	LoadConfigs(config_file);
}

Renderer::Renderer()
{

}

void Renderer::LoadConfigs(std::string config_file)
{
	std::vector<fs::path> configs = GetFilesInFolder(fs::path(config_file).parent_path().string(), ".cfg");
	sort(configs.begin(), configs.end());
	for (auto &file : configs)
	{
		rendering_configurations.push_back(file.filename().string());
	}
	config_folder = fs::path(config_file).parent_path().string();
}

void Renderer::ClearTextures()
{
	for (int i = 0; i < main_textures.size(); i++)
	{
		glDeleteTextures(1, &main_textures[i]);
	}

	for (int i = 0; i < shader_textures.size(); i++)
	{
		for (int j = 0; j < shader_textures[i].size(); j++)
		{
			if (!(i == shader_textures.size() - 1 && j == 0))
			{
				glDeleteTextures(1, &shader_textures[i][j]);
			}
		}
	}

	main_textures.clear();
	shader_textures.clear();
	global_size.clear();
}

void Renderer::ClearShaders()
{
	for (int i = 0; i < shader_pipeline.size(); i++)
	{
		shader_pipeline[i].Delete();
	}
	shader_pipeline.clear();
}


void Renderer::Initialize(int w, int h, std::string config_f)
{
	if (config_f == "")
	{
		config_f = config_file;
	}

	glUseProgram(0);
	ClearShaders();
	ClearTextures();

	width = w;
	height = h;

	variables["width"] = width;
	variables["height"] = height;
	camera.SetResolution(vec2(w, h));
	camera.SetAspectRatio((float)w / (float)h);

	ExprParser parser;
	config_file = config_f;
	std::string compute_folder = fs::path(config_file).parent_path().generic_string();

	std::ifstream config(config_file);
	if (config.fail())
	{
		ERROR_MSG("Error opening pipeline configuration");
		return;
	}
	std::string line;

	int element = -1;
	int cur_shader = 0;

	

	std::vector<GLuint> stage_textures;
	std::string shader_file;
	vec2 global, tex_resolution;
	while (std::getline(config, line))
	{
		if (line.substr(0, 1) != "#")
		{
			parser.Parse(line);
			switch (element++)
			{
			case -1:
				for (int i = 0; i < parser.Evaluate(variables); i++)
				{
					main_textures.push_back(GenerateTexture(width, height));
				}
				break;
			case 0:
				shader_file = compute_folder + "/" + line;
				LoadShader(shader_file);
				break;
			case 1:
				global.x = ceil(parser.Evaluate(variables));
				break;
			case 2:
				global.y = ceil(parser.Evaluate(variables));
				break;
			case 3:
				tex_resolution.x = ceil(parser.Evaluate(variables));
				break;
			case 4:
				tex_resolution.y = ceil(parser.Evaluate(variables));
				break;
			case 5:
				for (int i = 0; i < parser.Evaluate(variables); i++)
				{
					stage_textures.push_back(GenerateTexture(tex_resolution.x, tex_resolution.y));
				}
				shader_textures.push_back(stage_textures);
				stage_textures.clear();
				global_size.push_back(global);
				element = 0;
				break;
			}
		}
	}

	illumination_texture = GenerateTexture(width, height);
	weight_shader = ComputeShader(compute_folder + "/auto_exposure/auto_exposure_weighting.glsl");

	config.close();
}

void Renderer::ReInitialize(int w, int h)
{
	glUseProgram(0);
	ClearTextures();

	width = w;
	height = h;
	variables["width"] = width;
	variables["height"] = height;
	camera.SetResolution(vec2(w, h));
	camera.SetAspectRatio((float)w / (float)h);

	ExprParser parser;

	std::string compute_folder = fs::path(config_file).parent_path().generic_string();

	std::ifstream config(config_file);
	if (config.fail())
	{
		ERROR_MSG("Error opening pipeline configuration");
		return;
	}
	std::string line;

	int element = -1;
	int cur_shader = 0;

	std::vector<GLuint> stage_textures;
	std::string shader_file;
	vec2 global, tex_resolution;

	while (std::getline(config, line))
	{
		if (line.substr(0, 1) != "#")
		{
			parser.Parse(line);
			switch (element++)
			{
			case -1:
				for (int i = 0; i < parser.Evaluate(variables); i++)
				{
					main_textures.push_back(GenerateTexture(width, height));
				}
				break;
			case 0:
				//shader_file = compute_folder + "/" + line;
			//	LoadShader(shader_file);
				break;
			case 1:
				global.x = ceil(parser.Evaluate(variables));
				break;
			case 2:
				global.y = ceil(parser.Evaluate(variables));
				break;
			case 3:
				tex_resolution.x = ceil(parser.Evaluate(variables));
				break;
			case 4:
				tex_resolution.y = ceil(parser.Evaluate(variables));
				break;
			case 5:
				for (int i = 0; i < parser.Evaluate(variables); i++)
				{
					stage_textures.push_back(GenerateTexture(tex_resolution.x, tex_resolution.y));
				}
				shader_textures.push_back(stage_textures);
				stage_textures.clear();
				global_size.push_back(global);
				element = 0;
				break;
			}
		}
	}

	illumination_texture = GenerateTexture(width, height);

	config.close();
}


void Renderer::SetOutputTexture(sf::Texture & tex)
{
	shader_textures[shader_textures.size()-1][0] = tex.getNativeHandle();
}

void Renderer::LoadShader(std::string shader_file)
{
	shader_pipeline.push_back(ComputeShader(shader_file));
}

std::vector<std::string> Renderer::GetConfigurationsList()
{
	return rendering_configurations;
}

std::string Renderer::GetConfigFolder()
{
	return config_folder;
}

void Renderer::LoadExternalTextures(std::string texture_folder)
{
	std::vector<fs::path> images = GetFilesInFolder(texture_folder, ".png");
	for (auto &path : images)
	{
		sf::Texture textr; 
		textr.loadFromFile(path.string());
		input_textures.push_back(textr);
	}
}

void Renderer::Render()
{
	int stages = global_size.size();
	for (int i = 0; i < stages; i++)
	{
		int tex_id = 0;

		//bind textures from the previous step
		if (i != 0)
		{
			for (int j = 0; j < shader_textures[i - 1].size(); j++)
			{
				glBindImageTexture(tex_id++, shader_textures[i - 1][j], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
			}
		}

		//bind textures from the current step
		for (int j = 0; j < shader_textures[i].size(); j++)
		{
			glBindImageTexture(tex_id++, shader_textures[i][j], 0, GL_FALSE, 0, GL_READ_WRITE, (i == stages-1)?GL_RGBA8:GL_RGBA32F);
		}

		//global textures
		for (int j = 0; j < main_textures.size(); j++)
		{
			glBindImageTexture(tex_id++, main_textures[j], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		}

		//input textures
		int inp_tex = 0;
		for (auto &extr_text : input_textures)
		{
			shader_pipeline[i].setUniform(inp_tex++, extr_text.getNativeHandle());
		}

		glBindImageTexture(tex_id++, illumination_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		
		shader_pipeline[i].setCamera(camera.GetGLdata());
		shader_pipeline[i].Run(global_size[i]);
	}

	camera.UpdateExposure(EvaluateAvgIllumination());

	//increment frame number
	camera.Fpp();
}

float Renderer::EvaluateAvgIllumination()
{
	//if coordinate/depth map available 
	if (main_textures.size() > 0)
	{
		//precalculation
		glBindImageTexture(0, shader_textures[global_size.size() - 1][0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
		glBindImageTexture(1, main_textures[0], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F); //depth
		glBindImageTexture(2, illumination_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		weight_shader.setCamera(camera.GetGLdata());
		weight_shader.Run(vec2(ceil(width / 8.f), ceil(height / 8.f)));
	}

	if (main_textures.size() > 0)
	{
		glBindTexture(GL_TEXTURE_2D, illumination_texture);
	}
	else
	{
		//just use the final texture  
		glBindTexture(GL_TEXTURE_2D, shader_textures[global_size.size() - 1][0]);
	}

	//get the average of the texture using mipmaps
	float avg[4];
	int mipmap_level = floor(log2(float(std::max(width, height))));
	glGenerateMipmap(GL_TEXTURE_2D);
	glGetTexImage(GL_TEXTURE_2D, mipmap_level, GL_RGBA, GL_FLOAT, avg);
	GLenum err = glGetError();
	glBindTexture(GL_TEXTURE_2D, 0);

	if (main_textures.size() > 0)
	{
		return avg[0] / avg[1];
	}
	else
	{
		return sqrt(avg[0] * avg[0] + avg[1] * avg[1] + avg[2] * avg[2]);
	}
	
}

GLuint Renderer::GenerateTexture(float w, float h)
{
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	//HDR texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	return texture;
}
