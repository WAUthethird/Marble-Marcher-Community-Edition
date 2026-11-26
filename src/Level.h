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
#include <Renderer.h>
#include <fstream>
#include <vector>
#include <deque>
#include <Eigen/Dense>
#include <SFML/Audio.hpp>
#include <filesystem>
#include <map>
#include <algorithm>

#ifdef _WIN32
#include <Windows.h>
#define ERROR_MSG(x) MessageBox(nullptr, TEXT(x), TEXT("ERROR"), MB_OK);
#else
#include <iostream>
#define ERROR_MSG(x) std::cerr << x << std::endl;
#endif

namespace fs = std::filesystem;

static const int num_levels = 24;
static const int num_fractal_params = 9;
typedef Eigen::Matrix<float, num_fractal_params, 1> FractalParams;

struct LevelF
{
	int FractalIterations;
	float FractalParams[9];
	float PBR_roughness;	 //Fractal roughness
	float PBR_metal;         //Fractal metalicity

	float marble_rad;        //Radius of the marble
	float start_look_x;      //Camera direction on start
	float orbit_dist;        //Distance to orbit
	float start_pos[3];      //Starting position of the marble
	float end_pos[3];        //Ending goal flag position
	float kill_y;            //Below this height player is killed
	bool planet;             //True if gravity should be like a planet
	char txt[255];           //Title
	char desc[255];			 //Description
	float anim_1;            //Animation amount for angle1 parameter
	float anim_2;            //Animation amount for angle2 parameter
	float anim_3;            //Animation amount for offset_y parameter
	float light_dir[3];	     //Sun light direction
	float light_col[3];      //Sun light color
	float background_col[3];

	int level_id;	     	 //level identifier
	int link_level;			 //Play what level after finish
	char use_music[255];     //what track to use

	float ground_force;
	float air_force;
	float ground_friction;
	float air_friction;
	float gravity;
	float ground_ratio;

	float gravity_dir[3];

	//Advanced Animation
	float FractalParamsAmp[9];
	float FractalParamsFrq[9];
};

//Rebuild extensions: appended after LevelF in the .lvl file.
//Old levels (only LevelF) still load and get the defaults below.
static const int MAX_EXTRA_FLAGS = 16;
//Checkpoints have no upper limit. The first MAX_CHECKPOINTS are stored in LevelExt
//(file compatibility), all further ones in an extra block after LevelExt (LevelCPBlock).
static const int MAX_CHECKPOINTS = 16;
//the renderer draws the checkpoints nearest to the camera, up to this many
static const int MAX_DRAWN_CHECKPOINTS = 32;
static const char LEVEL_EXT_MAGIC[8] = "MMCEXT1";
static const char LEVEL_CP_MAGIC[8] = "MMCECP1";

//header of the extra checkpoint block; followed by count * float[3]
struct LevelCPBlock
{
	char magic[8];
	int count;
};

//how the fractal glow is colored
enum GlowMulti
{
	GLOW_SINGLE = 0,         //one color (original)
	GLOW_THREE_COLORS = 1,   //three structural bands of the fractal, each with its own color
	GLOW_RAINBOW = 2,        //one band, cycles through the spectrum over time and space
	GLOW_RAINBOW_BANDS = 3   //three bands in different hues + animated spectrum
};

enum SymmetryType
{
	SYM_MENGER = 0,      //cubic (original)
	SYM_SIERPINSKI = 1,  //tetrahedral
	SYM_DIAGONAL = 2,    //single diagonal plane
	SYM_NONE = 3         //only the mirror axes
};

struct LevelExt
{
	char magic[8];
	int version;

	//Fractal animation: rotation axes of Angle1 / Angle2
	float rot_axis1[3];
	float rot_axis2[3];

	//Fractal glow
	bool glow_enabled;
	bool glow_beat_sync;
	float glow_color[3];
	float glow_brightness;
	float glow_beat_strength;
	float glow_pattern[3];

	//Sky
	bool custom_sky;
	float sky_color[3];

	//Symmetry
	int sym_type;
	bool sym_mirror[3];
	int sym_radial_global;
	int sym_radial_internal;

	//Game rules
	bool jump_enabled;
	float jump_height;       //in marble radii
	bool rubber_ball;
	float rubber_bounce;     //0..1 restitution
	float ball_weight;
	bool kill_y_enabled;
	float max_fall_speed;    //marble radii per second, 0 = unlimited
	float max_speed;         //marble radii per second, 0 = unlimited

	//Multiple flags + checkpoints
	bool multi_flag;
	int num_flags;
	float flags[MAX_EXTRA_FLAGS][3];
	int num_checkpoints;
	float checkpoints[MAX_CHECKPOINTS][3];

	//Music reaction
	bool music_react;
	float music_react_strength;

	//beat analysis sensitivity equalizer (5 bands, see MusicAnalyzer.h)
	float beat_eq[5];

	//checkpoints must be hit in order before the flags (taken from the reserved block,
	//so levels saved before keep loading: they have 0 here)
	int ordered_checkpoints;
	float time_limit;        //seconds, 0 = no time limit

	//multi color glow (taken from the reserved block: old levels have 0 = single color)
	int glow_multi;          //GlowMulti
	float glow_color2[3];
	float glow_color3[3];
	float glow_hue_speed;    //rainbow cycles per second
	float glow_hue_spread;   //rainbow cycles per marble radius

	float reserved[21];
};

class Level {
public:
  Level() { InitExtDefaults(); }
  Level(float s, float a1, float a2,
        const Eigen::Vector3f& v,
        const Eigen::Vector3f& c,
        float rad,
        float look_x,
        float orbit_d,
        const Eigen::Vector3f& start,
        const Eigen::Vector3f& end,
        float kill,
        bool planet,
        const char* desc,
        float an1=0.0f, float an2=0.0f, float an3=0.0f);

  bool LoadFromFile(fs::path path);
  void SaveToFile(std::string file, int ID, int Link);

  void LoadLevelF(LevelF lvl);
  LevelF GetLevelF();

  int FractalIter;			 //Fractal iterations
  FractalParams params;      //Fractal parameters

  float PBR_roughness;		 //Fractal roughness
  float PBR_metal;           //Fractal metalicity

  float marble_rad;          //Radius of the marble
  float start_look_x;        //Camera direction on start
  float orbit_dist;          //Distance to orbit
  Eigen::Vector3f start_pos; //Starting position of the marble
  Eigen::Vector3f end_pos;   //Ending goal flag position
  float kill_y;              //Below this height player is killed
  bool planet;               //True if gravity should be like a planet
  std::string txt;           //Title
  std::string desc;			 //Description
  float anim_1;              //Animation amount for angle1 parameter
  float anim_2;              //Animation amount for angle2 parameter
  float anim_3;              //Animation amount for offset_y parameter
  Eigen::Vector3f light_dir; //Sun light direction
  Eigen::Vector3f light_col; //Sun light color

  int level_id;				 //level identifier
  int link_level;			 //Play what level after finish
  std::string use_music;     //what track to use

  Eigen::Vector3f background_col;

  float ground_force;
  float air_force;
  float ground_friction;
  float air_friction;
  float gravity;
  float ground_ratio;

  Eigen::Vector3f gravity_dir;

  //Advanced Animation
  FractalParams FractalParamsAmp;
  FractalParams FractalParamsFrq;

  //---- Rebuild extensions ----
  void InitExtDefaults();
  void LoadLevelExt(const LevelExt& ext);
  LevelExt GetLevelExt();

  Eigen::Vector3f rot_axis1;      //rotation axis for Angle1 (default Z)
  Eigen::Vector3f rot_axis2;      //rotation axis for Angle2 (default X)

  bool glow_enabled;
  bool glow_beat_sync;
  Eigen::Vector3f glow_color;
  float glow_brightness;
  float glow_beat_strength;
  Eigen::Vector3f glow_pattern;   //orbit trap that decides WHERE it glows
  int glow_multi;                 //GlowMulti
  Eigen::Vector3f glow_color2;
  Eigen::Vector3f glow_color3;
  float glow_hue_speed;
  float glow_hue_spread;

  bool custom_sky;
  Eigen::Vector3f sky_color;

  int sym_type;
  bool sym_mirror[3];
  int sym_radial_global;
  int sym_radial_internal;

  bool jump_enabled;
  float jump_height;
  bool rubber_ball;
  float rubber_bounce;
  float ball_weight;
  bool kill_y_enabled;
  float max_fall_speed;
  float max_speed;

  bool multi_flag;
  int num_flags;
  Eigen::Vector3f flags[MAX_EXTRA_FLAGS];
  //number of checkpoints (no upper limit). checkpoints.size() can be larger for a moment:
  //removed entries stay allocated until the editor bar no longer points to them (TrimCheckpoints).
  int num_checkpoints;
  //deque: adding checkpoints keeps the addresses of the existing ones valid (AntTweakBar holds pointers)
  std::deque<Eigen::Vector3f> checkpoints;
  void AddCheckpoint(const Eigen::Vector3f& p)
  {
	  if ((int)checkpoints.size() > num_checkpoints) checkpoints[num_checkpoints] = p;
	  else checkpoints.push_back(p);
	  num_checkpoints++;
  }
  void RemoveLastCheckpoint() { if (num_checkpoints > 0) num_checkpoints--; }
  void TrimCheckpoints() { if ((int)checkpoints.size() > num_checkpoints) checkpoints.resize(num_checkpoints); }

  bool music_react;
  float music_react_strength;
  float beat_eq[5];          //beat detection sensitivity per frequency band
  bool ordered_checkpoints;  //checkpoints in order, then the flags
  float time_limit;          //seconds, 0 = off; the ball is destroyed when it runs out

  bool HasDefaultSymmetry() const {
	  return sym_type == SYM_MENGER && sym_mirror[0] && sym_mirror[1] && sym_mirror[2] &&
		  sym_radial_global <= 1 && sym_radial_internal <= 1;
  }
};

extern Level all_levels[num_levels];
extern Level default_level;

std::vector<fs::path> GetFilesInFolder(std::string folder, std::string filetype);

struct Score
{
	int level_id = 0;
	int played_num = 0;
	float best_time = 0.f;
	float all_time = 0.f;
	float last_time = 0.f;
};

class All_Levels
{
public:
	All_Levels() {}

	void LoadLevelsFromFolder(std::string folder);
	void LoadMusicFromFolder(std::string folder);

	bool LevelExists(int ID);
	Level GetLevel(int ID);
	int GetLevelNum();
	std::map<int, std::string> getLevelNames();
	std::map<int, std::string> getLevelDesc();
	std::vector<int> getLevelIds();
	std::map<int, Score> getLevelScores();
	sf::Music* GetLevelMusic(int ID);

	sf::Music * GetMusic(std::string music);

	void RecreateMissing();

	void ReloadLevels();

	void LoadLevelFromFile(fs::path file);
	void LoadScoresFromFile(std::string file);
	float GetBest(int lvl);
	void SaveScoresToFile(const std::string& file);

	bool UpdateScore(int lvl, float time);
	void DeleteLevel(int lvl);
	//saves a copy of the level under a new, unused name and id; returns the new id
	int SaveAsNewLevel(Level lvl, std::string name);
	int DuplicateLevel(int lvl);
	sf::Music* GetMusicByID(int ID);

	void StopAllMusic();

	std::vector<std::string> GetMusicNames();
	std::string GetMusicPath(const std::string& name);
	//name of the currently playing track ("" if none) + its playback position in seconds
	std::string GetPlayingMusic(float& offset);

private:
	std::map<std::string, std::string> music_paths;
	std::map<int, Level> level_map;
	std::map<int, Score> score_map;
	std::map<int, int> level_id_map;
	std::map<int, std::string> level_names;
	std::map<int, std::string> level_descriptions;
	std::vector<int> level_ids;

	std::map<std::string, sf::Music*> music_map;
	std::vector<std::string> music_names;

	std::string lvl_folder;

	int level_num;
};

std::string ConvertSpaces2_(std::string text);
