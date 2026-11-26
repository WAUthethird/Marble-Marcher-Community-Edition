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

#include "Level.h"
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <Eigen/Dense>
#include<Settings.h>
#include "MusicAnalyzer.h"

#define MAX_DIST 20.f
#define MAX_MARCHES 1000
#define MIN_DIST 1e-4f
#define FOCAL_DIST 1.73205080757

extern sf::Music *current_music;
extern bool recording;
extern bool replay;

struct InputRecord
{
	float move_x, move_y;
	float view_x, view_y;
	float cam_z;
	bool mouse_clicked;
	bool jump;
};


class Scene {
public:
  enum CamMode {
    INTRO,
    SCREEN_SAVER,
    ORBIT,
    DEORBIT,
    MARBLE,
    GOAL,
    FINAL,
    MIDPOINT
  };

  enum EditorMode
  {
	  DEFAULT,
	  PLACE_MARBLE,
	  PLACE_FLAG,
	  PLACE_EXTRA_FLAG,
	  PLACE_CHECKPOINT
  };

  EditorMode cur_ed_mode;
  int ed_place_index;   //which extra flag / checkpoint is being placed
  FractalParams   frac_params;
  FractalParams   frac_params_smooth;

  Level           level_copy;
  All_Levels	  levels;

  bool PlayNext;
  bool PBR_Enabled;
  bool Refl_Refr_Enabled;
  bool Shadows_Enabled;
  bool Fog_Enabled;
  int Fractal_Iterations;
  float camera_size;
  float free_camera_speed;
  int MarbleType;
  Eigen::Vector3f LIGHT_DIRECTION;
  float PBR_METALLIC;
  float PBR_ROUGHNESS;

  float gamma_material;
  float gamma_sky;
  float gamma_camera;

  std::string original_level_name;

  float           marble_rad;
  Eigen::Vector3f marble_pos;
  Eigen::Vector3f marble_vel;
  Eigen::Matrix3f marble_mat;

  void SetCurrentMusic(sf::Music * new_music);

  void StopMusic();

  Scene(sf::Music* level_music);

  void LoadLevel(int level);
  void SetMarble(float x, float y, float z, float r);
  void SetMarbleScale(float r);
  void SetFlag(float x, float y, float z);
  void SetMode(CamMode mode);
  void SetResolution(int x, int y);
  void SetExposure(float e) { exposure = e; }
  void SetWindowResolution(int x, int y);
  void EnbaleCheats() { enable_cheats = true; }
  Eigen::Vector3f GetVelocity();

  const Eigen::Vector3f& GetMarble() const { return marble_pos; };
  float GetCamLook() const { return cam_look_x_smooth; }
  float GetCamLookX() const { return cam_look_x; }
  float GetMarbleScale() const { return marble_rad; }
  const Eigen::Vector3f& GetFlagPos() const { return flag_pos; }
  CamMode GetMode() const { return cam_mode; }
  int GetLevel() const { return cur_level; }
  int GetCountdownTime() const;
  int GetSumTime() const { return sum_time; }
  sf::Vector3f GetGoalDirection() const;
  bool IsSinglePlay() const { return play_single; }
  bool IsHighScore();
  bool IsFullRun() const { return is_fullrun && !enable_cheats; }
  bool IsFreeCamera() const { return free_camera; }
  bool HasCheats() const { return enable_cheats; }
  int GetParamMod() const { return param_mod; }

  sf::Music& GetCurMusic() const;
  void StopAllMusic();

  void StartNewGame();
  void StartNextLevel();
  void ReplayLevel(int level);
  void StartSingle(int level);
  void StartLevelEditor(int level);
  void ResetCamera();
  void StartDefault();
  void ResetLevel();
  void ResetCheats();
  void Synchronize();

  void UpdateMarble(float dx=0.0f, float dy=0.0f);
  void UpdateCamera(float dx=0.0f, float dy=0.0f, float dz=0.0f, bool speedup=false);

  void SnapCamera();
  void HideObjects();

  //---- Rebuild extensions ----
  //rendering faster than the 60 Hz simulation: interpolate between the last two ticks
  void BeginTick() { prev_cam_mat = cam_mat; prev_marble_pos = marble_pos; }
  float render_alpha;
  Eigen::Vector3f RenderMarblePos() const;
  void Jump() { jump_requested = true; }

  //level editor markers (labels drawn on top of the 3D view)
  struct EditorMarker { Eigen::Vector3f pos; std::string label; sf::Color color; bool selected; };
  std::vector<EditorMarker> GetEditorMarkers() const;
  bool ProjectToScreen(const Eigen::Vector3f& p, float& sx, float& sy) const;
  void RestartOrRespawn(bool full);
  void EditorPlace(int mousex, int mousey);
  bool IsMultiFlag() const { return level_copy.multi_flag && level_copy.num_flags > 0; }
  //ordered checkpoints: index of the next checkpoint to hit (num_checkpoints when all are done)
  int GetNextCheckpoint() const;
  bool CheckpointsPending() const { return level_copy.ordered_checkpoints && GetNextCheckpoint() < level_copy.num_checkpoints; }
  int GetCheckpointTotal() const { return level_copy.num_checkpoints; }
  //time limit: remaining frames (-1 = no limit)
  int GetTimeLeft() const;
  //split time display (frames to show, label, time in frames)
  int split_show;
  int split_time;
  std::string split_label;
  int GetNextFlag() const { return next_flag; }               //0-based index in the flag sequence
  int GetFlagTotal() const { return level_copy.num_flags + 1; }
  Eigen::Vector3f GetTargetFlagPos() const;
  Eigen::Vector3f GravityUp() const;
  //checkpoint progress that survives closing the level
  bool HasSavedProgress(int level);
  void ClearSavedProgress(int level);
  void RestoreSavedProgress();          //call right after StartSingle()
  bool IsRestoredRun() const { return restored_run; }

  //crush effects (glass shards / metal dumpling)
  static const int MAX_SHARDS = 24;
  bool IsCrushing() const { return crush_timer > 0; }

  float music_beat;      //0..1 beat pulse of the playing music (with the level equalizer)
  float music_energy;    //0..1 loudness of the playing music
  bool  on_ground;

  void Write(sf::Shader& shader) const;
  void WriteLVL(int lvl)
  {
	  cur_level = lvl;
  }

  void WriteRenderer(Renderer & rd);

  void WriteShader(ComputeShader & rd);

  float DE(const Eigen::Vector3f& pt) const;
  Eigen::Vector3f NP(const Eigen::Vector3f& pt) const;
  bool MarbleCollision(float& delta_v);

  void Cheat_ColorChange();
  void Cheat_FreeCamera();
  void Cheat_Gravity();
  void Cheat_HyperSpeed();
  void Cheat_IgnoreGoal();
  void Cheat_Motion();
  void Cheat_Planet();
  void Cheat_Zoom();
  void Cheat_Param(int param);

  void ExitEditor();

  Eigen::Vector3f MouseRayCast(int mousex, int mousey, float min_dist = MIN_DIST);
  Eigen::Vector3f RayMarch(const Eigen::Vector3f& pt, const Eigen::Vector3f& ray, float min_dist = MIN_DIST);

  sf::Sound sound_goal;
  sf::SoundBuffer buff_goal;
  sf::Sound sound_bounce1;
  sf::SoundBuffer buff_bounce1;
  sf::Sound sound_bounce2;
  sf::SoundBuffer buff_bounce2;
  sf::Sound sound_bounce3;
  sf::SoundBuffer buff_bounce3;
  sf::Sound sound_shatter;
  sf::SoundBuffer buff_shatter;
  sf::Sound sound_rubber;          //synthesized "boing" of the rubber ball
  sf::SoundBuffer buff_rubber;

protected:
  void SetLevel(int level);

  void UpdateIntro(bool ssaver);
  void UpdateOrbit();
  void UpdateDeOrbit(float dx, float dy, float dz, int iters);
  void UpdateNormal(float dx, float dy, float dz);
  void UpdateCameraOnly(float dx, float dy, float dz);
  void UpdateGoal();
  void MakeCameraRotation();

  void UpdateMusic();
  void CheckFlags();
  bool TouchesFlag(const Eigen::Vector3f& fpos) const;
  void ReachGoal();
  void Die();
  void ResetProgress();
  Eigen::Vector3f FlagSequencePos(int i) const; //multi flag order: main flag first
  Eigen::Vector3f CheckpointSpawn(int i) const;
  Eigen::Vector3f FlagSpawn(const Eigen::Vector3f& fpos) const;
  void SetRespawn(const Eigen::Vector3f& p, const std::string& label);
  void SaveProgress();
  std::string ProgressFile(int level);
  void StartCrush(const Eigen::Vector3f& dir);
  void UpdateCrush();
  bool FlagsFlipped() const { return level_copy.planet || level_copy.gravity < 0.0f; }
  Eigen::Vector3f NPGradient(const Eigen::Vector3f& pt, float& de, Eigen::Vector3f& n) const;

private:
  float           time;
  int             cur_level;
  bool            is_fullrun;
  bool            intro_needs_snap;
  bool            play_single;
  bool			  level_editor;

  Eigen::Matrix4f cam_mat;
  float           cam_look_x;
  float           cam_look_y;
  float           cam_dist;
  Eigen::Vector3f cam_pos;
  CamMode         cam_mode;

  float           cam_look_x_smooth;
  float           cam_look_y_smooth;
  float           cam_dist_smooth;
  Eigen::Vector3f cam_pos_smooth;

  Eigen::Vector3f flag_pos;


  int			  ResX, ResY;
  int			  WinX, WinY;
  int             timer;
  int             final_time;
  int             sum_time;
  float           exposure;



  sf::Music* music;


  bool            enable_cheats;
  bool            free_camera;
  int             gravity_type;
  int             param_mod;
  bool            ignore_goal;
  bool            hyper_speed;
  bool            disable_motion;
  bool            zoom_to_scale;
  float			  gravity;

  //rebuild state
  Eigen::Matrix4f prev_cam_mat;
  Eigen::Vector3f prev_marble_pos;
  bool            jump_requested;
  int             jump_cooldown;
  bool            marble_dead;
  float           flip_angle;      //camera roll for inverted gravity
  float           squash;          //rubber ball deformation
  float           squash_vel;
  Eigen::Vector3f squash_dir;
  int             next_flag;
  int             active_checkpoint;
  std::vector<char> cp_active;      //one entry per checkpoint, no upper limit
  bool CpActive(int i) const { return i >= 0 && i < (int)cp_active.size() && cp_active[i]; }
  void SetCpActive(int i) { if (i < 0) return; if (i >= (int)cp_active.size()) cp_active.resize(i + 1, 0); cp_active[i] = 1; }

  //saved progress
  struct SavedProgress { int checkpoint; int next_flag; int timer; unsigned int cp_mask; float respawn[3]; int has_respawn; };
  bool            has_respawn;      //respawn at the last reached checkpoint / flag
  Eigen::Vector3f respawn_pos;
  bool            time_over;
  bool            pending_restore;
  SavedProgress   restore_data;
  std::vector<char> restore_cp;     //hit checkpoints of the saved progress (all of them, not only the first 32)
  bool            restored_run;

  //crush effect state
  int             crush_timer;     //frames left, 0 = not crushing
  bool            crush_metal;
  Eigen::Vector3f crush_dir;
  Eigen::Vector3f shard_pos[MAX_SHARDS];
  Eigen::Vector3f shard_vel[MAX_SHARDS];
  Eigen::Vector3f shard_axis[MAX_SHARDS];
  float           shard_angle[MAX_SHARDS];
  float           shard_spin[MAX_SHARDS];
  float           shard_size[MAX_SHARDS];
};

int * GetReplayFrame();

void StartRecording();

void StopRecording2File(std::string path, bool save = true);

void StartReplayFromFile(std::string path);
void StopReplay();