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

namespace fs = std::filesystem;

class Settings {
public:
  Settings() :
    mute(false),
    mouse_sensitivity(0) {
  }

  void Load(const std::string& fname) {
    std::ifstream fin(fname, std::ios::binary);
    if (!fin) { return; }
    fin.read((char*)this, sizeof(this));
  }
  void Save(const std::string& fname) {
    std::ofstream fout(fname, std::ios::binary);
    if (!fout) { return; }
    fout.write((char*)this, sizeof(this));
  }

  bool   mute;
  int    mouse_sensitivity;
};

class AdditionalSettings
{
public:
	int screenshot_width;
	int screenshot_height;
	std::string lang;

	AdditionalSettings() :
		screenshot_width(1920),
		screenshot_height(1080),
		lang("English")
	{}

	void Load(const std::string& fname) {
		int increment = 0;
		std::ifstream config;
		config.open(fname);
		if (config.fail())
		{
			return;
		}
		std::string line;
		while (getline(config, line))
		{
			if (line.substr(0, 1) != "#")
			{
				increment++;
				std::istringstream iss(line);
				float num;
				while ((iss >> num))
				{
					switch (increment)
					{
					case 1:
						screenshot_width = num;
						break;
					case 2:
						screenshot_height = num;
						break;
					case 4:
						
						break;
					default:
						break;
					}
				}
				if (increment == 3)
				{
					lang = line;
				}
			}
		}
	}
};

struct MainSettings
{
	sf::Vector2i rendering_resolution;
	sf::Vector2i screenshot_resolution;
	int MRRM_scale;
	int shadow_resolution;
	int bloom_resolution;
	std::string language;

	bool shadows;
	bool refl_refr;
	bool fog;

	float bloom_intensity;
	float bloom_treshold;
	float bloom_radius;
	float gamma;
	float FOV;

	float music_volume;
	float fx_volume;
	float mouse_sensitivity;

	bool loop_level;
};

static const MainSettings default_settings = { sf::Vector2i(1280, 800),
	sf::Vector2i(2560, 1440), 4, 2, 2, "English", true, true, true, 0.05, 2.7, 3, 2.2, 75, 1, 1, 0.005, false };


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

	void Load(std::string settings_file)
	{
		if (!LoadFromFile(settings_file))
		{
			stg = default_settings;
		}
	}

	bool LoadFromFile(std::string settings_file)
	{
		filename = settings_file;
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

	bool new_settings;

	MainSettings stg;
	std::string filename;

	~AllSettings()
	{
		SaveToFile(filename);
	}
};

extern AllSettings SETTINGS;