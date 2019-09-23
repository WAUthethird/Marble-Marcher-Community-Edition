/* This file is part of the Marble Marcher (https://github.com/HackerPoet/MarbleMarcher).
* Copyright(C) 2018 CodeParade
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once
#include <fstream>
#include <sstream>
#include <SFML/Graphics.hpp>
#include <filesystem>
#include <array>
#include <AntTweakBar.h>
namespace fs = std::filesystem;

const int num_of_keys = 14;
enum KEYS {
	UP, DOWN, LEFT, RIGHT, VIEWUP, VIEWDOWN, VIEWLEFT, VIEWRIGHT, 
	JOYSTICK_MOVE_AXIS_X, JOYSTICK_MOVE_AXIS_Y, JOYSTICK_VIEW_AXIS_X, JOYSTICK_VIEW_AXIS_Y, JOYSTICK_EXIT, JOYSTICK_SCREENSHOT
};

struct MainSettings
{
	int rendering_resolution;
	int screenshot_resolution;
	int MRRM_scale;
	int shadow_resolution;
	int bloom_resolution;
	int language;

	bool shadows;
	bool refl_refr;
	bool fog;

	float bloom_intensity;
	float bloom_radius;
	float gamma;
	float FOV;

	float music_volume;
	float fx_volume;
	float mouse_sensitivity;
	float wheel_sensitivity;

	bool loop_level;

	float motion_blur;
	float exposure;
	int shader_config;

	bool fullscreen;
	bool VSYNC;
	int marble_type;
	bool play_next;

	float gamma_material;
	float gamma_sky;
	float gamma_camera;

	bool cross_eye;
	float eye_separation;
	float auto_exposure_speed; 
	float auto_exposure_target;
	bool touch_mode;

	std::array<int, num_of_keys> control_mapping;
	
	//cheeets
	bool speed_regulation;
};

extern TwEnumVal resolutions[];
static const std::array<int, num_of_keys> default_control_mapping = { sf::Keyboard::W, sf::Keyboard::S, sf::Keyboard::A, sf::Keyboard::D,
														sf::Keyboard::Up, sf::Keyboard::Down, sf::Keyboard::Left, sf::Keyboard::Right,
														1, 2, 3, 4, 1, 2};
//an incomprehensible wall of parameters 
static const MainSettings default_settings = 
{ 
	6, 10, 5, 2, 3, 0, true, true, true, 0.06, 9, 2.2, 70, 20, 20, 0.005, 0.2, false,
	0.005, 0.45, 0, false, true, 0, true, 0.7, 0.6, 2.2, false, -0.02, 0.2, 0.55, 
	false, default_control_mapping, true 
};


class AllSettings
{
public:
	AllSettings()
	{
		stg = default_settings;
	}

	AllSettings(std::string settings_file)
	{
		Load(settings_file);
	}

	bool Load(std::string settings_file)
	{
		if (!LoadFromFile(settings_file))
		{
			stg = default_settings;	
			first_start = true;
			return true;
		}
		first_start = false;
		return false;
	}

	bool LoadFromFile(std::string settings_file)
	{
		filename = settings_file;
		if (!fs::exists(settings_file))
		{
			return false;
		}

		int cfg_size = fs::file_size(settings_file);
		int MainSettings_size = sizeof(MainSettings);

		if (cfg_size != MainSettings_size)
		{
			return false;
		}

		std::ifstream cfg_file(settings_file, std::ios_base::in | std::ios_base::binary);

		cfg_file.seekg(0);
		cfg_file.read(reinterpret_cast<char *>(&stg), sizeof(MainSettings));

		cfg_file.close();
	}

	void SaveToFile(std::string settings_file)
	{
		std::ofstream cfg_file(settings_file, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);

		cfg_file.write(reinterpret_cast<char *>(&stg), sizeof(MainSettings));

		cfg_file.close();
	}

	MainSettings stg;
	std::string filename;

	bool first_start;

	~AllSettings()
	{
		SaveToFile(filename);
	}
};

extern AllSettings SETTINGS;