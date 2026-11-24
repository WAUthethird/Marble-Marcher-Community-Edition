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
#include "Scene.h"
#include "Res.h"
#include <iostream>
#include <cstring>

static const float PI = 3.14159265359f;
static const float ground_force = 0.008f;
static const float air_force = 0.004f;
static const float ground_friction = 0.99f;
static const float air_friction = 0.995f;
static const float orbit_speed = 0.005f;
static const int max_marches = 10;
static const int num_phys_steps = 6;
static const float marble_bounce = 1.2f; //Range 1.0 to 2.0
static const float orbit_smooth = 0.995f;
static const float zoom_smooth = 0.85f;
static const float look_smooth = 0.75f;
static const float look_smooth_free_camera = 0.9f;
static const int frame_transition = 400;
static const int frame_orbit = 600;
static const int frame_deorbit = 800;
static const int frame_countdown = frame_deorbit + 3*60;
static const float default_zoom = 15.0f;
static const float ground_ratio = 1.15f;
static const int mus_switches[num_level_music] = {9, 15, 21, 24};
static const int num_levels_midpoint = 15;

static void MakeRubberSound(sf::SoundBuffer& buffer);

int cur_frame = 0;
bool recording = false;
bool replay = false;

std::vector<InputRecord> recording_data;
std::fstream input_recording;

int* GetReplayFrame()
{
	return &cur_frame;
}

void StartRecording()
{
	if (!replay)
	{
		recording_data.clear();
		recording = true;
	}
}

void StopRecording2File(std::string path, bool save)
{
	if (recording)
	{
		recording = false;
		if (save)
		{
			input_recording.open(path, std::ios::out | std::ios::binary | std::ios::trunc);

			if (!input_recording.is_open())
			{
				//DisplayError("Error opening record file");
			}
			else
			{
				for (auto &input : recording_data)
				{
					input_recording.write((char*)&input, sizeof(InputRecord));
				}
			}

			input_recording.close();
		}

		recording_data.clear();
	}
}

void StartReplayFromFile(std::string path)
{
	if (!recording)
	{
		recording_data.clear();
		replay = true;
		input_recording.open(path, std::ios::in | std::ios::binary);

		if (!input_recording.is_open())
		{
			replay = false;
			//DisplayError("Error opening record file");
		}
		else
		{
			input_recording.seekg(0, input_recording.beg);
			InputRecord rec;
			while (input_recording)
			{
				input_recording.read((char*)&rec, sizeof(InputRecord));
				recording_data.push_back(rec);
			}
			cur_frame = 0;
		}

		input_recording.close();
	}
}

void StopReplay()
{
	if (!recording)
	{
		replay = false;
		recording_data.clear();
	}
}


void SaveRecord(float mx, float my, float vx, float vy, float cz, bool mc, bool jump)
{
	if (recording && !replay)
	{
		InputRecord rec;
		memset(&rec, 0, sizeof(InputRecord));
		rec.jump = jump;
		rec.move_x = mx;
		rec.move_y = my;
		rec.view_x = vx;
		rec.view_y = vy;
		rec.cam_z = cz;
		rec.mouse_clicked = mc;

		recording_data.push_back(rec);
	}
}

void AddResetPadding()
{
	for (int i = 0; i < frame_deorbit; i++)
	{
		SaveRecord(0, 0, 0, 0, 0, 0, false);
	}
}

InputRecord GetRecord()
{
	if (replay)
	{
		if (cur_frame >= recording_data.size())
		{
			cur_frame = 0;
			replay = false;
			recording_data.clear();
			return InputRecord();
		}
		return recording_data[cur_frame++];
	}
	return InputRecord();
}



sf::Music *current_music = nullptr;

void Scene::SetCurrentMusic(sf::Music *new_music)
{
	if (current_music != new_music)
	{
		StopMusic();
		current_music = new_music;
		if (current_music == nullptr) return; //level music file missing
		current_music->play();
		current_music->setVolume(SETTINGS.stg.music_volume);
	}
}

void Scene::StopMusic()
{
	if (current_music != nullptr)
		current_music->stop();
}

static void ModPi(float& a, float b) {
  if (a - b > PI) {
    a -= 2 * PI;
  } else if (a - b < -PI) {
    a += 2 * PI;
  }
}

Scene::Scene(sf::Music* level_music) :
	intro_needs_snap(true),
	play_single(false),
	is_fullrun(false),
	exposure(1.0f),
	cam_mat(Eigen::Matrix4f::Identity()),
	cam_look_x(0.0f),
	cam_look_y(0.0f),
	cam_dist(default_zoom),
	cam_pos(0.0f, 0.0f, 0.0f),
	cam_mode(CamMode::INTRO),
	marble_rad(1.0f),
	marble_pos(0.0f, 0.0f, 0.0f),
	marble_vel(0.0f, 0.0f, 0.0f),
	marble_mat(Eigen::Matrix3f::Identity()),
	flag_pos(0.0f, 0.0f, 0.0f),
	timer(0),
	sum_time(0),
	music(level_music),
	cur_level(0),
	PBR_Enabled(1),
	Refl_Refr_Enabled(1),
	Shadows_Enabled(1),
	Fractal_Iterations(16),
	MarbleType(0),
	free_camera_speed(1.f),
    LIGHT_DIRECTION(Eigen::Vector3f(-0.36, 0.8, 0.48)),
	PBR_METALLIC(0.5),
	PBR_ROUGHNESS(0.4),
	camera_size(0.075),
	cur_ed_mode(DEFAULT),
	level_editor(false),
	gravity(0.005f),
	time(0.f),
	ed_place_index(0),
	music_beat(0.f),
	music_energy(0.f),
	on_ground(false),
	jump_requested(false),
	jump_cooldown(0),
	marble_dead(false),
	flip_angle(0.f),
	squash(0.f),
	squash_vel(0.f),
	squash_dir(0.f, -1.f, 0.f),
	next_flag(0),
	active_checkpoint(-1),
	pending_restore(false),
	restored_run(false),
	crush_timer(0),
	crush_metal(false),
	crush_dir(0.f, -1.f, 0.f),
	split_show(0),
	split_time(0),
	has_respawn(false),
	respawn_pos(0.f, 0.f, 0.f),
	time_over(false),
	render_alpha(1.f),
	prev_cam_mat(Eigen::Matrix4f::Identity()),
	prev_marble_pos(0.f, 0.f, 0.f)
{
  for (int i = 0; i < MAX_CHECKPOINTS; i++) cp_active[i] = false;
  ResetCheats();
  frac_params.setOnes();
  frac_params_smooth.setOnes();
  SnapCamera();
  buff_goal.loadFromFile(goal_wav);
  sound_goal.setBuffer(buff_goal);
  buff_bounce1.loadFromFile(bounce1_wav);
  sound_bounce1.setBuffer(buff_bounce1);
  buff_bounce2.loadFromFile(bounce2_wav);
  sound_bounce2.setBuffer(buff_bounce2);
  buff_bounce3.loadFromFile(bounce3_wav);
  sound_bounce3.setBuffer(buff_bounce3);
  buff_shatter.loadFromFile(shatter_wav);
  sound_shatter.setBuffer(buff_shatter);
  MakeRubberSound(buff_rubber);
  sound_rubber.setBuffer(buff_rubber);
}
Eigen::Vector3f Scene::GetVelocity() {
	return marble_vel;
}

void Scene::LoadLevel(int level) {
  SetLevel(level);
  marble_pos = level_copy.start_pos;
  marble_rad = level_copy.marble_rad;
  flag_pos = level_copy.end_pos;
  cam_look_x = level_copy.start_look_x;
}

void Scene::SetMarble(float x, float y, float z, float r) {
  marble_rad = r;
  marble_pos = Eigen::Vector3f(x, y, z);
  marble_vel.setZero();
}
void Scene::SetMarbleScale(float r) {
  marble_rad = r;
}

void Scene::SetFlag(float x, float y, float z) {
  flag_pos = Eigen::Vector3f(x, y, z);
}

void Scene::SetLevel(int level) {
  cur_level = level;    
  level_copy = levels.GetLevel(level);
  SetCurrentMusic(levels.GetLevelMusic(level));
  MUSIC_ANALYZER.Request(level_copy.use_music, levels.GetMusicPath(level_copy.use_music));
  flip_angle = 0.f;
  ResetProgress();
}

void Scene::ResetProgress()
{
	next_flag = 0;
	active_checkpoint = -1;
	for (int i = 0; i < MAX_CHECKPOINTS; i++) cp_active[i] = false;
	marble_dead = false;
	jump_requested = false;
	jump_cooldown = 0;
	squash = squash_vel = 0.f;
	pending_restore = false;
	restored_run = false;
	crush_timer = 0;
	has_respawn = false;
	time_over = false;
	split_show = 0;
}

int Scene::GetNextCheckpoint() const
{
	for (int i = 0; i < level_copy.num_checkpoints; i++)
	{
		if (!cp_active[i]) return i;
	}
	return level_copy.num_checkpoints;
}

int Scene::GetTimeLeft() const
{
	if (level_copy.time_limit <= 0.0f) return -1;
	return std::max(int(level_copy.time_limit * 60.0f) - timer, 0);
}

Eigen::Vector3f Scene::FlagSpawn(const Eigen::Vector3f& fpos) const
{
	Eigen::Vector3f up = level_copy.planet ? fpos.normalized() : Eigen::Vector3f(0.f, 1.f, 0.f);
	if (FlagsFlipped() && !level_copy.planet) up = -up;
	if (level_copy.planet && level_copy.gravity < 0.0f) up = -up;
	return fpos + up * (1.5f * level_copy.marble_rad);
}

void Scene::SetRespawn(const Eigen::Vector3f& p, const std::string& label)
{
	has_respawn = true;
	respawn_pos = p;
	//split time: total time since the start (it keeps running after respawns)
	split_label = label;
	split_time = timer;
	split_show = 150;
	SaveProgress();
}

//R / right mouse: back to the last checkpoint or flag, full restart if there is none
void Scene::RestartOrRespawn(bool full)
{
	if (!full && has_respawn && cam_mode == MARBLE && !time_over)
	{
		crush_timer = 0;
		marble_dead = false;
		marble_pos = respawn_pos;
		marble_vel.setZero();
		squash = squash_vel = 0.f;
		return;
	}
	ResetLevel();
}

void Scene::SetMode(CamMode mode) {
  //Don't reset the timer if transitioning to screen saver
  if ((cam_mode == INTRO && mode == SCREEN_SAVER) ||
      (cam_mode == SCREEN_SAVER && mode == INTRO)) {
  } else {
    timer = 0;
    intro_needs_snap = true;
  }
  cam_mode = mode;
}

void Scene::SetResolution(int x, int y)
{
	ResX = x;
	ResY = y;
}

void Scene::SetWindowResolution(int x, int y)
{
	WinX = x;
	WinY = y;
}

int Scene::GetCountdownTime() const {
  if (cam_mode == DEORBIT && timer >= frame_deorbit) {
    return timer - frame_deorbit;
  } else if (cam_mode == MARBLE) {
    return timer + 3*60;
  } else if (cam_mode == GOAL) {
    return final_time + 3*60;
  } else {
    return -1;
  }
}

sf::Vector3f Scene::GetGoalDirection() const {
  Eigen::Vector3f goal_delta = marble_mat.transpose() * (GetTargetFlagPos() - marble_pos);
  goal_delta.y() = 0.0f;
  const float goal_dir = std::atan2(-goal_delta.z(), goal_delta.x());
  const float a = cam_look_x - goal_dir;
  const float b = std::abs(cam_look_y * 2.0f / PI);
  const float d = goal_delta.norm() / marble_rad;
  return sf::Vector3f(a, b, d);
}

sf::Music& Scene::GetCurMusic() const {
  for (int i = 0; i < num_level_music; ++i) {
    if (cur_level < mus_switches[i]) {
      return music[i];
    }
  }
  return music[0];
}

void Scene::StopAllMusic() {
  for (int i = 0; i < num_level_music; ++i) {
    music[i].stop();
  }
  levels.StopAllMusic();
}

bool Scene::IsHighScore(){
  if (cam_mode != GOAL) {
    return false;
  } else {
	  float best = levels.GetBest(cur_level);
    return best == final_time / 60.f;
  }
}

void Scene::StartNewGame() {
  sum_time = 0;
  play_single = false;
  ResetCheats();
  is_fullrun = true;
  SetLevel(0);
  HideObjects();
  StartRecording();
  SetMode(ORBIT);
}

void Scene::StartNextLevel() {
  if (play_single) {
    cam_mode = MARBLE;
	if (PlayNext && levels.LevelExists(level_copy.link_level))
	{
		SetLevel(level_copy.link_level);
		HideObjects();
		SetMode(ORBIT);
		StartRecording();
	}
	else
	{
		ResetLevel();
	}
    
  } else if (cur_level + 1 == num_levels_midpoint && cam_mode != MIDPOINT) {
    cam_mode = MIDPOINT;
  } else if (cur_level + 1 >= num_levels) {
    cam_mode = FINAL;
  } else {
    SetLevel(cur_level + 1);
    HideObjects();
    SetMode(ORBIT);
	StartRecording();
  }
}

void Scene::ReplayLevel(int level)
{
	StopRecording2File("", 0);
	play_single = true;
	is_fullrun = false;
	ResetCheats();
	SetLevel(level);
	StartReplayFromFile(std::string(recordings_folder) + "/" + ConvertSpaces2_(level_copy.txt) + ".bin");
	HideObjects();
	SetMode(ORBIT);
}

void Scene::StartSingle(int level) {
  play_single = true;
  is_fullrun = false;
  ResetCheats();
  SetLevel(level);
  StartRecording();
  HideObjects();
  SetMode(ORBIT);
}



void Scene::StartLevelEditor(int level)
{
	if (level < 0)
	{
		cur_level = -1;
		level_copy = default_level;
	}
	else
	{
		cur_level = level;
		level_copy = levels.GetLevel(level);
	}
	original_level_name = level_copy.txt;
	play_single = true;
	is_fullrun = false;
	ResetCheats();
	level_editor = true;
	ResetProgress();
	flip_angle = 0.f;
	SetMode(ORBIT);
	enable_cheats = false;
	free_camera = true;
}

void Scene::ResetCamera()
{
	SetMode(DEORBIT);
	timer = frame_deorbit;
	free_camera_speed = 1.f;
}

void Scene::StartDefault()
{
	cur_level = -1;
	level_copy = default_level;
}

void Scene::ResetLevel() {
  if (cam_mode == MARBLE || play_single) {
    StopRecording2File("", false);
	StartRecording();
	AddResetPadding();
	ResetProgress();
    SetMode(DEORBIT);
    timer = frame_deorbit;
    frac_params = level_copy.params;
    frac_params_smooth = frac_params;
    marble_pos = level_copy.start_pos;
    marble_vel.setZero();
    marble_rad = level_copy.marble_rad;
    marble_mat.setIdentity();
    flag_pos = level_copy.end_pos;
    cam_look_x = level_copy.start_look_x;
    cam_look_x_smooth = cam_look_x;
    cam_pos = cam_pos_smooth;
    cam_dist = default_zoom;
    cam_dist_smooth = cam_dist;
    cam_look_y = -0.3f;
    cam_look_y_smooth = cam_look_y;
  }
}

void Scene::ResetCheats() {
  enable_cheats = false;
  free_camera = false;
  gravity_type = 0;
  param_mod = -1;
  ignore_goal = false;
  hyper_speed = false;
  disable_motion = false;
  zoom_to_scale = false;
}

void Scene::Synchronize()
{
	for (int i = 0; i < 9; i++)
	{
		 frac_params_smooth[i] = frac_params[i] = level_copy.params[i];
	}
}

void Scene::UpdateCamera(float dx, float dy, float dz, bool speedup) {
	time += 1.f / 60.f;
	UpdateMusic();
	//Camera update depends on current mode
  gravity = level_copy.gravity;
  const int iters = speedup ? 5 : 1;
  if (cam_mode == INTRO) {
    UpdateIntro(false);
  } else if (cam_mode == SCREEN_SAVER) {
    UpdateIntro(true);
  } else if (cam_mode == ORBIT) {
    for (int i = 0; i < iters; i++) {
      UpdateOrbit();
      if (cam_mode != ORBIT) {
        break;
      }
    }
  } else if (cam_mode == DEORBIT) {
    //For deorbit, speedup is handled by the function called, not by calling the function multiple times
    UpdateDeOrbit(dx, dy, dz, iters);
  } else if (cam_mode == MARBLE) {
    UpdateNormal(dx, dy, dz);
  } else if (cam_mode == GOAL || cam_mode == FINAL || cam_mode == MIDPOINT) {
    for (int i = 0; i < iters; i++) {
      UpdateGoal();
      if (cam_mode != GOAL) {
        break;
      }
    }
  }
}

//================= Saved checkpoint progress =================
std::string Scene::ProgressFile(int level) {
  return std::string(level_folder) + "/progress/" + std::to_string(level) + ".cp";
}

bool Scene::HasSavedProgress(int level) {
  std::error_code ec;
  return fs::exists(ProgressFile(level), ec);
}

void Scene::ClearSavedProgress(int level) {
  std::error_code ec;
  fs::remove(ProgressFile(level), ec);
}

void Scene::SaveProgress() {
  if (level_editor || !has_respawn || replay) { return; }
  std::error_code ec;
  fs::create_directories(std::string(level_folder) + "/progress", ec);
  SavedProgress sp;
  sp.checkpoint = active_checkpoint;
  sp.next_flag = next_flag;
  sp.timer = timer;
  sp.cp_mask = 0;
  for (int i = 0; i < MAX_CHECKPOINTS; i++) { if (cp_active[i]) sp.cp_mask |= (1u << i); }
  sp.respawn[0] = respawn_pos.x(); sp.respawn[1] = respawn_pos.y(); sp.respawn[2] = respawn_pos.z();
  sp.has_respawn = 1;
  std::ofstream f(ProgressFile(cur_level), std::ios::binary | std::ios::trunc);
  f.write(reinterpret_cast<const char*>(&sp), sizeof(sp));
}

void Scene::RestoreSavedProgress() {
  std::ifstream f(ProgressFile(cur_level), std::ios::binary);
  SavedProgress sp;
  memset(&sp, 0, sizeof(sp));
  f.read(reinterpret_cast<char*>(&sp), sizeof(sp));
  if (f.gcount() < 16) { return; }  //older files only have the first 4 fields
  if (!sp.has_respawn) {
    if (sp.checkpoint < 0 || sp.checkpoint >= level_copy.num_checkpoints) { return; }
    const Eigen::Vector3f p = CheckpointSpawn(sp.checkpoint);
    sp.respawn[0] = p.x(); sp.respawn[1] = p.y(); sp.respawn[2] = p.z();
    sp.has_respawn = 1;
  }
  restore_data = sp;
  pending_restore = true;
}

Eigen::Vector3f Scene::CheckpointSpawn(int i) const {
  const Eigen::Vector3f cp = level_copy.checkpoints[i];
  if (level_copy.planet) {
    Eigen::Vector3f up = cp.normalized();
    if (level_copy.gravity < 0.0f) { up = -up; }
    return cp + up * (1.5f * level_copy.marble_rad);
  }
  const float s = FlagsFlipped() ? -level_copy.marble_rad : level_copy.marble_rad;
  return cp + Eigen::Vector3f(0.0f, 1.5f * s, 0.0f);
}

//multi flag order: the main flag is flag 1, then the extra flags; the last one finishes
Eigen::Vector3f Scene::FlagSequencePos(int i) const {
  return (i <= 0) ? flag_pos : level_copy.flags[std::min(i - 1, MAX_EXTRA_FLAGS - 1)];
}

//================= Crush effects =================
static float Rand01() { return float(rand()) / float(RAND_MAX); }

void Scene::StartCrush(const Eigen::Vector3f& dir) {
  sound_shatter.play();
  crush_timer = 180; //3 seconds
  crush_metal = (MarbleType == 1) && !level_copy.rubber_ball;
  crush_dir = dir;
  if (crush_metal) {
    //squashed like a dumpling
    squash_dir = dir;
    squash = 0.72f;
    squash_vel = 0.0f;
  } else {
    //physical shards flying apart
    for (int i = 0; i < MAX_SHARDS; i++) {
      Eigen::Vector3f r(Rand01() * 2 - 1, Rand01() * 2 - 1, Rand01() * 2 - 1);
      if (r.norm() < 1e-3f) { r = Eigen::Vector3f(0, 1, 0); }
      r.normalize();
      shard_pos[i] = marble_pos + r * (marble_rad * 0.6f * Rand01());
      shard_vel[i] = marble_vel * 0.5f + (r - dir * 0.8f) * (marble_rad * (0.03f + 0.06f * Rand01()));
      Eigen::Vector3f ax(Rand01() * 2 - 1, Rand01() * 2 - 1, Rand01() * 2 - 1);
      shard_axis[i] = (ax.norm() > 1e-3f) ? ax.normalized() : Eigen::Vector3f(1, 0, 0);
      shard_angle[i] = Rand01() * 2 * PI;
      shard_spin[i] = (Rand01() * 2 - 1) * 0.4f;
      shard_size[i] = marble_rad * (0.18f + 0.22f * Rand01());
    }
  }
  marble_vel.setZero();
}

void Scene::UpdateCrush() {
  if (!crush_metal) {
    const Eigen::Vector3f g = -GravityUp() * (marble_rad * std::abs(gravity));
    for (int i = 0; i < MAX_SHARDS; i++) {
      Eigen::Vector3f gi = g;
      if (level_copy.planet) {
        gi = -shard_pos[i].normalized() * (marble_rad * gravity);
      }
      shard_vel[i] += gi;
      shard_vel[i] *= 0.995f;
      shard_pos[i] += shard_vel[i];
      shard_angle[i] += shard_spin[i];
      //collide with the fractal
      const float r = shard_size[i] * 0.5f;
      const float d = DE(shard_pos[i]);
      if (d < r) {
        const float h = std::max(r * 0.1f, 1e-6f);
        Eigen::Vector3f n(
          DE(shard_pos[i] + Eigen::Vector3f(h, 0, 0)) - DE(shard_pos[i] - Eigen::Vector3f(h, 0, 0)),
          DE(shard_pos[i] + Eigen::Vector3f(0, h, 0)) - DE(shard_pos[i] - Eigen::Vector3f(0, h, 0)),
          DE(shard_pos[i] + Eigen::Vector3f(0, 0, h)) - DE(shard_pos[i] - Eigen::Vector3f(0, 0, h)));
        n = (n.norm() > 1e-12f) ? n.normalized() : GravityUp();
        shard_pos[i] += n * (r - d);
        const float vn = shard_vel[i].dot(n);
        if (vn < 0.0f) {
          shard_vel[i] -= n * (vn * 1.35f);   //small bounce
          shard_vel[i] *= 0.8f;               //friction
          shard_spin[i] *= 0.7f;
        }
      }
    }
  }
  if (--crush_timer <= 0) {
    crush_timer = 0;
    Die();
  }
}

static void MakeRubberSound(sf::SoundBuffer& buffer) {
  //synthesized "boing": short attack, falling pitch with wobble, decaying
  const unsigned int rate = 44100;
  const float len = 0.42f;
  std::vector<sf::Int16> samples(size_t(rate * len));
  float phase = 0.0f;
  for (size_t i = 0; i < samples.size(); i++) {
    const float t = float(i) / rate;
    const float freq = (170.0f + 330.0f * std::exp(-t * 11.0f)) * (1.0f + 0.07f * std::sin(2.0f * PI * 19.0f * t) * std::exp(-t * 4.0f));
    phase += 2.0f * PI * freq / rate;
    const float env = (1.0f - std::exp(-t * 600.0f)) * std::exp(-t * 7.5f);
    const float v = std::sin(phase) + 0.35f * std::sin(2.0f * phase) + 0.12f * std::sin(3.0f * phase);
    samples[i] = sf::Int16(std::max(-1.0f, std::min(1.0f, 0.55f * v * env)) * 32000.0f);
  }
  buffer.loadFromSamples(samples.data(), samples.size(), 1, rate);
}

Eigen::Vector3f Scene::GravityUp() const {
  Eigen::Vector3f up = level_copy.planet ? marble_pos.normalized() : Eigen::Vector3f(0.0f, 1.0f, 0.0f);
  if (level_copy.gravity < 0.0f) { up = -up; }
  return up;
}

Eigen::Vector3f Scene::GetTargetFlagPos() const {
  if (CheckpointsPending()) {
    return level_copy.checkpoints[GetNextCheckpoint()];
  }
  if (IsMultiFlag()) {
    return FlagSequencePos(next_flag);
  }
  //without the multi flag mode every flag finishes the level => point to the closest one
  Eigen::Vector3f best = flag_pos;
  float best_d = (flag_pos - marble_pos).squaredNorm();
  for (int i = 0; i < level_copy.num_flags; i++) {
    const float d = (level_copy.flags[i] - marble_pos).squaredNorm();
    if (d < best_d) { best_d = d; best = level_copy.flags[i]; }
  }
  return best;
}

void Scene::UpdateMusic() {
  float offset = 0.f;
  const std::string name = levels.GetPlayingMusic(offset);
  if (name.empty()) {
    music_beat *= 0.9f;
    music_energy *= 0.9f;
    return;
  }
  MUSIC_ANALYZER.Request(name, levels.GetMusicPath(name));
  music_beat = MUSIC_ANALYZER.GetBeat(name, offset, level_copy.beat_eq);
  music_energy = MUSIC_ANALYZER.GetEnergy(name, offset);
}

bool Scene::TouchesFlag(const Eigen::Vector3f& fpos) const {
  const bool flip = FlagsFlipped();
  const bool flag_y_match = flip ?
    marble_pos.y() <= fpos.y() && marble_pos.y() >= fpos.y() - 7 * marble_rad :
    marble_pos.y() >= fpos.y() && marble_pos.y() <= fpos.y() + 7 * marble_rad;
  if (!flag_y_match) { return false; }
  const float fx = marble_pos.x() - fpos.x();
  const float fz = marble_pos.z() - fpos.z();
  return fx*fx + fz*fz < 6 * marble_rad*marble_rad;
}

void Scene::ReachGoal() {
  final_time = timer;
  bool best = false;
  ClearSavedProgress(cur_level);
  //a run continued from a saved checkpoint does not count as a record
  if (!enable_cheats && !restored_run) {
    best = levels.UpdateScore(cur_level, final_time / 60.f);
  }
  StopRecording2File(std::string(recordings_folder) + "/" + ConvertSpaces2_(level_copy.txt) + ".bin", best);
  SetMode(GOAL);
  sound_goal.setPitch(1.0f);
  sound_goal.play();
}

void Scene::CheckFlags() {
  //checkpoints first (in the ordered mode only the next one can be hit)
  const int next_cp = GetNextCheckpoint();
  for (int i = 0; i < level_copy.num_checkpoints; i++) {
    if (cp_active[i] && active_checkpoint == i) { continue; }
    if (level_copy.ordered_checkpoints && !cp_active[i] && i != next_cp) { continue; }
    const float s = FlagsFlipped() ? -marble_rad : marble_rad;
    const Eigen::Vector3f mid = level_copy.checkpoints[i] + Eigen::Vector3f(0.0f, 2.4f * s, 0.0f);
    if ((marble_pos - mid).norm() < 3.0f * marble_rad) {
      const bool is_new = !cp_active[i];
      if (is_new) {
        sound_goal.setPitch(2.0f);
        sound_goal.play();
      }
      cp_active[i] = true;
      active_checkpoint = i;
      if (is_new) {
        SetRespawn(CheckpointSpawn(i), "Checkpoint " + std::to_string(i + 1));
      } else {
        respawn_pos = CheckpointSpawn(i);
        has_respawn = true;
        SaveProgress();
      }
    }
  }

  //the flags stay locked until all ordered checkpoints are done
  if (CheckpointsPending()) { return; }

  if (IsMultiFlag()) {
    //only the current flag can be activated, all others are locked
    //main flag = flag 1, extra flags follow, the last one completes the level
    if (TouchesFlag(FlagSequencePos(next_flag))) {
      if (next_flag >= level_copy.num_flags) {
        ReachGoal();
        return;
      }
      //a reached flag is also a respawn point
      SetRespawn(FlagSpawn(FlagSequencePos(next_flag)), "Flag " + std::to_string(next_flag + 1));
      next_flag++;
      sound_goal.setPitch(1.5f);
      sound_goal.play();
    }
  } else {
    bool reached = TouchesFlag(flag_pos);
    for (int i = 0; i < level_copy.num_flags && !reached; i++) {
      reached = TouchesFlag(level_copy.flags[i]);
    }
    if (reached) {
      ReachGoal();
      return;
    }
  }
}

void Scene::Die() {
  marble_dead = false;
  crush_timer = 0;
  if (time_over) {
    //the time limit ran out => the whole level starts again
    ResetLevel();
    return;
  }
  if (has_respawn) {
    //respawn at the last checkpoint / flag, keep the time and the progress
    marble_pos = respawn_pos;
    marble_vel.setZero();
    squash = squash_vel = 0.f;
    return;
  }
  ResetLevel();
}

void Scene::UpdateMarble(float dx, float dy) {
  //Ignore other modes
  if (cam_mode != MARBLE) {
    jump_requested = false;
    return;
  }
  if (split_show > 0) { split_show--; }
  if (crush_timer > 0) {
    jump_requested = false;
    UpdateCrush();
    return;
  }

  //Normalize force if too big
  const float mag2 = dx*dx + dy*dy;
  if (mag2 > 1.0f) {
    const float mag = std::sqrt(mag2);
    dx /= mag;
    dy /= mag;
  }

  if (free_camera) {
    cam_pos += cam_mat.block<3,1>(0,2) * (-marble_rad * dy * 0.5f * free_camera_speed);
    cam_pos += cam_mat.block<3, 1>(0,0) * (marble_rad * dx * 0.5f * free_camera_speed);
    cam_pos_smooth = cam_pos_smooth*0.9f + cam_pos*0.1f;
  } else {
    //Apply all physics (gravity and collision)
    bool onGround = false;
    float max_delta_v = 0.0f;
    for (int i = 0; i < num_phys_steps; ++i) {
      float force = marble_rad * gravity / num_phys_steps;
      if (gravity_type == 1) { force *= 0.25f; } else if (gravity_type == 2) { force *= 4.0f; }
      if (level_copy.planet) {
        marble_vel -= marble_pos.normalized() * force;
      } else {
        marble_vel.y() -= force;
      }
      marble_pos += marble_vel / num_phys_steps;
      onGround |= MarbleCollision(max_delta_v);
      if (marble_dead || crush_timer > 0) { break; }
    }
    on_ground = onGround;

    if (crush_timer > 0) { return; }

    //Play bounce sound if needed (the rubber ball has its own "boing")
    const float pitch = 1.0f;
    float bounce_delta_v = max_delta_v / marble_rad;
    if (level_copy.rubber_ball) {
      if (bounce_delta_v > 0.06f) {
        const float w = std::max(level_copy.ball_weight, 0.01f);
        sound_rubber.setPitch(std::min(std::max(1.15f / std::pow(w, 0.2f) + 0.15f * bounce_delta_v, 0.4f), 2.5f));
        sound_rubber.setVolume(SETTINGS.stg.fx_volume * std::min(1.0f, 0.25f + bounce_delta_v / 0.4f));
        sound_rubber.play();
      }
    } else if (bounce_delta_v > 0.5f) {
      sound_bounce1.setPitch(pitch);
      sound_bounce1.play();
    } else if (bounce_delta_v > 0.25f) {
      sound_bounce2.setPitch(pitch);
      sound_bounce2.play();
    } else if (bounce_delta_v > 0.1f) {
      sound_bounce3.setPitch(pitch);
      sound_bounce3.setVolume(SETTINGS.stg.fx_volume * (bounce_delta_v / 0.25f));
      sound_bounce3.play();
    }

    //Add force from keyboard (steering acceleration from the level, heavier = slower)
    const float weight = std::max(level_copy.ball_weight, 0.01f);
    float f = marble_rad * (onGround ? level_copy.ground_force : level_copy.air_force) / weight;
    if (hyper_speed) { f *= 4.0f; }
    const float cs = std::cos(cam_look_x);
    const float sn = std::sin(cam_look_x);
    const Eigen::Vector3f v(dx*cs - dy*sn, 0.0f, -dy*cs - dx*sn);
    marble_vel += (marble_mat * v) * f;

    //Jump
    if (jump_cooldown > 0) { jump_cooldown--; }
    if (jump_requested && level_copy.jump_enabled && onGround && jump_cooldown == 0) {
      const Eigen::Vector3f up = GravityUp();
      float g = std::abs(gravity);
      if (gravity_type == 1) { g *= 0.25f; } else if (gravity_type == 2) { g *= 4.0f; }
      //v = sqrt(2 g h), everything measured in marble radii
      const float jv = marble_rad * std::sqrt(2.0f * g * std::max(level_copy.jump_height, 0.0f));
      const float vu = marble_vel.dot(up);
      if (vu < jv) { marble_vel += up * (jv - vu); }
      jump_cooldown = 12;
    }
    jump_requested = false;

    //Apply friction
    marble_vel *= (onGround ? level_copy.ground_friction : level_copy.air_friction);

    //Speed limits (given in marble radii per second)
    if (level_copy.max_fall_speed > 0.0f) {
      const Eigen::Vector3f down = -GravityUp();
      const float vmax = level_copy.max_fall_speed * marble_rad / 60.0f;
      const float vd = marble_vel.dot(down);
      if (vd > vmax) { marble_vel -= down * (vd - vmax); }
    }
    if (level_copy.max_speed > 0.0f) {
      const float vmax = level_copy.max_speed * marble_rad / 60.0f;
      const float vn = marble_vel.norm();
      if (vn > vmax) { marble_vel *= vmax / vn; }
    }
  }

  //Rubber ball deformation: damped spring
  if (level_copy.rubber_ball) {
    squash_vel += -0.35f * squash;
    squash_vel *= 0.78f;
    squash += squash_vel;
    squash = std::min(std::max(squash, -0.35f), 0.6f);
  } else {
    squash = squash_vel = 0.f;
  }

  //Update animated fractals
  if (!disable_motion) {
    frac_params[1] = level_copy.params[1] + level_copy.anim_1 * std::sin(timer * 0.015f);
    frac_params[2] = level_copy.params[2] + level_copy.anim_2 * std::sin(timer * 0.015f);
    frac_params[4] = level_copy.params[4] + level_copy.anim_3 * std::sin(timer * 0.015f);
  }
  //Fractal pulses with the music
  if (level_copy.music_react && !disable_motion) {
    frac_params[0] = level_copy.params[0] * (1.0f + 0.012f * level_copy.music_react_strength * music_beat);
  } else {
    frac_params[0] = level_copy.params[0];
  }
  frac_params_smooth = frac_params;

  //Time limit: the ball is destroyed when the timer runs out
  if (level_copy.time_limit > 0.0f && !time_over && !level_editor && timer >= int(level_copy.time_limit * 60.0f)) {
    time_over = true;
    if (!SETTINGS.stg.crush_effects_off) {
      StartCrush(-GravityUp());
    } else {
      sound_shatter.play();
      Die();
    }
    return;
  }

  //Check if marble has hit a flag or a checkpoint
  if (cam_mode != GOAL && !ignore_goal && !marble_dead) {
    CheckFlags();
    if (cam_mode == GOAL) { return; }
  }

  //Crushed by the fractal
  if (marble_dead) {
    Die();
    return;
  }

  //Check if marble passed the death barrier
  //(with negative gravity "down" is +y, so the barrier is mirrored)
  if (enable_cheats) {
    if (marble_pos.y() < -999.0f || marble_pos.y() > 999.0f) { ResetLevel(); }
  } else if (level_copy.kill_y_enabled) {
    const bool inverted = (level_copy.gravity < 0.0f) && !level_copy.planet;
    const bool below = inverted ? (-marble_pos.y() < level_copy.kill_y) : (marble_pos.y() < level_copy.kill_y);
    if (below) { Die(); }
  }
}

void Scene::UpdateIntro(bool ssaver) {
  //Update the timer
  const float t = -2.0f + timer * 0.002f;
  timer += 1;

  //Get rotational parameters
  const float dist = (ssaver ? 10.0f : 8.0f);
  const Eigen::Vector3f orbit_pt(0.0f, 3.0f, 0.0f);
  const Eigen::Vector3f perp_vec(std::sin(t), 0.0f, std::cos(t));
  cam_pos = orbit_pt + perp_vec * dist;
  cam_pos_smooth = cam_pos_smooth*0.9f + cam_pos*0.1f;

  //Solve for the look direction
  cam_look_x = std::atan2(perp_vec.x(), perp_vec.z());
  if (!ssaver) { cam_look_x += 0.5f; }
  ModPi(cam_look_x_smooth, cam_look_x);
  cam_look_x_smooth = cam_look_x_smooth*0.9f + cam_look_x*0.1f;

  //Update look y
  cam_look_y = (ssaver ? -0.25f : -0.41f);
  cam_look_y_smooth = cam_look_y_smooth*0.9f + cam_look_y*0.1f;

  //Update the camera matrix
  marble_mat.setIdentity();
  MakeCameraRotation();
  cam_mat.block<3, 1>(0, 3) = cam_pos_smooth;

  //Update demo fractal
  frac_params[0] = 1.6f;
  frac_params[1] = 2.0f + 0.5f*std::cos(timer * 0.0021f);
  frac_params[2] = PI + 0.5f*std::cos(timer * 0.000287f);
  frac_params[3] = -4.0f + 0.5f*std::sin(timer * 0.00161f);
  frac_params[4] = -1.0f + 0.1f*std::sin(timer * 0.00123f);
  frac_params[5] = -1.0f + 0.1f*std::cos(timer * 0.00137f);
  frac_params[6] = -0.2f;
  frac_params[7] = -0.1f;
  frac_params[8] = -0.6f;
  frac_params_smooth = frac_params;

  //Make sure marble and flag are hidden
  HideObjects();

  if (intro_needs_snap) {
    SnapCamera();
    intro_needs_snap = false;
  }
}

void Scene::UpdateOrbit() {
  //Update the timer
  const float t = timer * orbit_speed;
  float a = std::min(float(timer) / float(frame_transition), 1.0f);
  a *= a/(2*a*(a - 1) + 1);
  timer += 1;
  sum_time += 1;

  //Get marble location and rotational parameters
  const float orbit_dist = level_copy.orbit_dist;
  const Eigen::Vector3f orbit_pt(0.0f, orbit_dist, 0.0f);
  const Eigen::Vector3f perp_vec(std::sin(t), 0.0f, std::cos(t));
  cam_pos = orbit_pt + perp_vec * (orbit_dist * 2.5f);
  cam_pos_smooth = cam_pos_smooth*orbit_smooth + cam_pos*(1 - orbit_smooth);

  //Solve for the look direction
  cam_look_x = std::atan2(cam_pos_smooth.x(), cam_pos_smooth.z());
  ModPi(cam_look_x_smooth, cam_look_x);
  cam_look_x_smooth = cam_look_x_smooth*(1 - a) + cam_look_x*a;

  //Update look smoothing
  cam_look_y = -0.3f;
  cam_look_y_smooth = cam_look_y_smooth*orbit_smooth + cam_look_y*(1 - orbit_smooth);

  //Update the camera matrix
  marble_mat.setIdentity();
  MakeCameraRotation();
  cam_mat.block<3, 1>(0, 3) = cam_pos_smooth;

  //Update fractal parameters
  ModPi(frac_params[1], level_copy.params[1]);
  ModPi(frac_params[2], level_copy.params[2]);
  frac_params_smooth = frac_params * (1.0f - a) + level_copy.params * a;

  //When done transitioning display the marble and flag
  if (timer >= frame_transition) {
    marble_pos = pending_restore ? Eigen::Vector3f(restore_data.respawn[0], restore_data.respawn[1], restore_data.respawn[2]) : level_copy.start_pos;
    marble_rad = level_copy.marble_rad;
    flag_pos = level_copy.end_pos;
  }

  //When done transitioning, setup level
  if (timer >= frame_orbit) {
    frac_params = level_copy.params;
    cam_look_x = cam_look_x_smooth;
    cam_pos = cam_pos_smooth;
    cam_dist = default_zoom;
    cam_dist_smooth = cam_dist;
    cam_mode = DEORBIT;
  }
}

void Scene::UpdateDeOrbit(float dx, float dy, float dz, int iters) {
  if (timer > frame_deorbit + 1) {
    for (int i = 0; i < iters; i++) {
      //Update the timer
      timer += 1;
      sum_time += 1;

      if (cam_mode != DEORBIT) {
        break;
      }
    }
    //Camera during countdown is not sped up
    UpdateCameraOnly(dx, dy, dz);
  } else {
    for (int i = 0; i < iters; i++) {
      //Update the timer
      const float t = timer * orbit_speed;
      float b = std::min(float(std::max(timer - frame_orbit, 0)) / float(frame_deorbit - frame_orbit), 1.0f);
      b *= b/(2*b*(b - 1) + 1);
      timer += 1;
      sum_time += 1;

      //Get marble location and rotational parameters
      const float orbit_dist = level_copy.orbit_dist;
      const Eigen::Vector3f orbit_pt(0.0f, orbit_dist, 0.0f);
      const Eigen::Vector3f perp_vec(std::sin(t), 0.0f, std::cos(t));
      const Eigen::Vector3f orbit_cam_pos = orbit_pt + perp_vec * (orbit_dist * 2.5f);
      cam_pos = cam_pos*orbit_smooth + orbit_cam_pos*(1 - orbit_smooth);

      //Solve for the look direction
      const float start_look_x = level_copy.start_look_x;
      cam_look_x = std::atan2(cam_pos.x(), cam_pos.z());
      ModPi(cam_look_x, start_look_x);

      //Solve for the look direction
      cam_look_x_smooth = cam_look_x*(1 - b) + start_look_x*b;

      //Update look smoothing
      cam_look_y = -0.3f;
      cam_look_y_smooth = cam_look_y_smooth*orbit_smooth + cam_look_y*(1 - orbit_smooth);

      //Update the camera rotation matrix
      MakeCameraRotation();

      //Update the camera position
      Eigen::Vector3f marble_cam_pos = marble_pos + cam_mat.block<3, 3>(0, 0) * Eigen::Vector3f(0.0f, 0.0f, marble_rad * cam_dist_smooth);
      marble_cam_pos += Eigen::Vector3f(0.0f, marble_rad * cam_dist_smooth * 0.1f, 0.0f);
      cam_pos_smooth = cam_pos*(1 - b) + marble_cam_pos*b;
      cam_mat.block<3, 1>(0, 3) = cam_pos_smooth;

      //Required for a smooth transition later on
      cam_look_x = cam_look_x_smooth;
      cam_look_y = cam_look_y_smooth;

      if (cam_mode != DEORBIT) {
        break;
      }
    }
  }

  //When done deorbiting, transition to play
  if (timer > frame_countdown) {
    cam_mode = MARBLE;
    cam_pos = cam_pos_smooth;
    timer = 0;
    if (pending_restore) {
      //continue from the saved checkpoint
      pending_restore = false;
      restored_run = true;
      timer = restore_data.timer;
      next_flag = std::min(std::max(restore_data.next_flag, 0), level_copy.num_flags);
      active_checkpoint = restore_data.checkpoint;
      has_respawn = true;
      respawn_pos = Eigen::Vector3f(restore_data.respawn[0], restore_data.respawn[1], restore_data.respawn[2]);
      for (int i = 0; i < MAX_CHECKPOINTS; i++) cp_active[i] = (restore_data.cp_mask >> i) & 1u;
    }
  }
}

void Scene::UpdateCameraOnly(float dx, float dy, float dz) {
  //Update camera zoom
  if (param_mod >= 0) {
    const float new_param = level_copy.params[param_mod] + dz*0.01f;
    level_copy.params[param_mod] = frac_params_smooth[param_mod] = frac_params[param_mod] = new_param;
  } else if (zoom_to_scale) {
    level_copy.marble_rad *= std::pow(2.0f, -dz);
    level_copy.marble_rad = std::min(std::max(level_copy.marble_rad, 0.0006f), 0.6f);
    marble_rad = marble_rad*zoom_smooth + level_copy.marble_rad*(1 - zoom_smooth);
  } else {
    cam_dist *= std::pow(2.0f, -dz);
    cam_dist = std::min(std::max(cam_dist, 5.0f), 30.0f);
  }
  cam_dist_smooth = cam_dist_smooth*zoom_smooth + cam_dist*(1 - zoom_smooth);

  //Update look direction
  cam_look_x += dx;
  cam_look_y += dy;
  cam_look_y = std::min(std::max(cam_look_y, -PI / 2), PI / 2);
  while (cam_look_x > PI) { cam_look_x -= 2 * PI; }
  while (cam_look_x < -PI) { cam_look_x += 2 * PI; }

  //Update look smoothing
  const float a = (free_camera ? look_smooth_free_camera : look_smooth);
  ModPi(cam_look_x_smooth, cam_look_x);
  cam_look_x_smooth = cam_look_x_smooth*a + cam_look_x*(1 - a);
  cam_look_y_smooth = cam_look_y_smooth*a + cam_look_y*(1 - a);

  //Setup rotation matrix for planets
  if (level_copy.planet) {
    marble_mat.col(1) = GravityUp();
    marble_mat.col(2) = -marble_mat.col(1).cross(marble_mat.col(0)).normalized();
    marble_mat.col(0) = -marble_mat.col(2).cross(marble_mat.col(1)).normalized();
  } else {
    //negative gravity => the whole view smoothly rolls upside down
    const float target = (level_copy.gravity < 0.0f) ? PI : 0.0f;
    flip_angle += (target - flip_angle) * 0.06f;
    if (std::abs(target - flip_angle) < 1e-4f) flip_angle = target;
    marble_mat = Eigen::AngleAxisf(flip_angle, Eigen::Vector3f::UnitX()).toRotationMatrix();
  }

  //Update the camera matrix
  MakeCameraRotation();
  if (!free_camera) {
    cam_pos = marble_pos + cam_mat.block<3, 3>(0, 0) * Eigen::Vector3f(0.0f, 0.0f, marble_rad * cam_dist_smooth);
    cam_pos += marble_mat.col(1) * (marble_rad * cam_dist_smooth * 0.1f);
    cam_pos_smooth = cam_pos;
  }
  cam_mat.block<3, 1>(0, 3) = cam_pos_smooth;
}

void Scene::UpdateNormal(float dx, float dy, float dz) {
  //Update camera
  UpdateCameraOnly(dx, dy, dz);

  //Update timer
  timer += 1;
  sum_time += 1;
}

void Scene::UpdateGoal() {
  //Update the timer
  const float t = timer * 0.01f;
  float a = std::min(t / 75.0f, 1.0f);
  timer += 1;
  if (cur_level != num_levels_midpoint - 1 && cur_level != num_levels - 1) {
    sum_time += 1;
  }

  //Get marble location and rotational parameters
  const float flag_dist = marble_rad * 6.5f;
  const Eigen::Vector3f orbit_pt = flag_pos + marble_mat * Eigen::Vector3f(0.0f, flag_dist, 0.0f);
  const Eigen::Vector3f perp_vec = Eigen::Vector3f(std::sin(t), 0.0f, std::cos(t));
  cam_pos = orbit_pt + marble_mat * perp_vec * (flag_dist * 3.5f);
  cam_pos_smooth = cam_pos_smooth*(1 - a) + cam_pos*a;

  //Solve for the look direction
  cam_look_x = std::atan2(perp_vec.x(), perp_vec.z());
  ModPi(cam_look_x_smooth, cam_look_x);
  cam_look_x_smooth = cam_look_x_smooth*(1 - a) + cam_look_x*a;

  //Update look smoothing
  cam_look_y = -0.25f;
  cam_look_y_smooth = cam_look_y_smooth*0.99f + cam_look_y*(1 - 0.99f);

  //Update the camera matrix
  MakeCameraRotation();
  cam_mat.block<3, 1>(0, 3) = cam_pos_smooth;

  //Animate marble
  marble_vel += (orbit_pt - marble_pos) * 0.005f;
  marble_pos += marble_vel;
  if (marble_vel.norm() > marble_rad*0.02f) {
    marble_vel *= 0.95f;
  }

  if (timer > 300 && cam_mode != FINAL && cam_mode != MIDPOINT) {
    StartNextLevel();
  }
}

void Scene::MakeCameraRotation() {
  cam_mat.setIdentity();
  const Eigen::AngleAxisf aa_x_smooth(cam_look_x_smooth, Eigen::Vector3f::UnitY());
  const Eigen::AngleAxisf aa_y_smooth(cam_look_y_smooth, Eigen::Vector3f::UnitX());
  cam_mat.block<3, 3>(0, 0) = marble_mat * (aa_x_smooth * aa_y_smooth).toRotationMatrix();
}

void Scene::SnapCamera() {
  cam_look_x_smooth = cam_look_x;
  cam_look_y_smooth = cam_look_y;
  cam_dist_smooth = cam_dist;
  cam_pos_smooth = cam_pos;
}

void Scene::HideObjects() {
  marble_pos = Eigen::Vector3f(999.0f, 999.0f, 999.0f);
  flag_pos = Eigen::Vector3f(999.0f, 999.0f, 999.0f);
  marble_vel.setZero();
}

void Scene::Write(sf::Shader& shader) const {
  shader.setUniform("iMat", sf::Glsl::Mat4(cam_mat.data()));

  if (level_editor)
  {
	  shader.setUniform("iMarblePos", sf::Glsl::Vec3(level_copy.start_pos.x(), level_copy.start_pos.y(), level_copy.start_pos.z()));
	  shader.setUniform("iFlagPos",  sf::Glsl::Vec3(level_copy.end_pos.x(), level_copy.end_pos.y(), level_copy.end_pos.z()));
  }
  else
  {
	  shader.setUniform("iMarblePos", free_camera ?
		  sf::Glsl::Vec3(999.0f, 999.0f, 999.0f) :
		  sf::Glsl::Vec3(marble_pos.x(), marble_pos.y(), marble_pos.z())
	  );
	  shader.setUniform("iFlagPos", free_camera ?
		  sf::Glsl::Vec3(-999.0f, -999.0f, -999.0f) :
		  sf::Glsl::Vec3(flag_pos.x(), flag_pos.y(), flag_pos.z())
	  );
  }

  if (cam_mode != INTRO)
  {
	  shader.setUniform("LIGHT_DIRECTION", sf::Glsl::Vec3(level_copy.light_dir[0], level_copy.light_dir[1], level_copy.light_dir[2]));
	  shader.setUniform("PBR_ENABLED", PBR_Enabled);
	  shader.setUniform("PBR_METALLIC", level_copy.PBR_metal);
	  shader.setUniform("PBR_ROUGHNESS", level_copy.PBR_roughness);
  }
  else
  {
	  shader.setUniform("LIGHT_DIRECTION", sf::Glsl::Vec3(LIGHT_DIRECTION[0], LIGHT_DIRECTION[1], LIGHT_DIRECTION[2]));
	  shader.setUniform("PBR_ENABLED", PBR_Enabled);
	  shader.setUniform("PBR_METALLIC", PBR_METALLIC);
	  shader.setUniform("PBR_ROUGHNESS", PBR_ROUGHNESS);
  }

  shader.setUniform("BACKGROUND_COLOR", sf::Glsl::Vec3(level_copy.background_col[0], level_copy.background_col[1], level_copy.background_col[2]));
  shader.setUniform("LIGHT_COLOR", sf::Glsl::Vec3(level_copy.light_col[0], level_copy.light_col[1], level_copy.light_col[2]));

  shader.setUniform("iMarbleRad", level_copy.marble_rad);

  shader.setUniform("iFlagScale", level_copy.planet ? -level_copy.marble_rad : level_copy.marble_rad);

  shader.setUniform("iFracScale", frac_params_smooth[0]);
  shader.setUniform("iFracAng1", frac_params_smooth[1]);
  shader.setUniform("iFracAng2", frac_params_smooth[2]);
  shader.setUniform("iFracShift", sf::Glsl::Vec3(frac_params_smooth[3], frac_params_smooth[4], frac_params_smooth[5]));
  shader.setUniform("iFracCol", sf::Glsl::Vec3(frac_params_smooth[6], frac_params_smooth[7], frac_params_smooth[8]));

  shader.setUniform("iExposure", exposure);


  shader.setUniform("SHADOWS_ENABLED", Shadows_Enabled);
  shader.setUniform("CAMERA_SIZE", camera_size*level_copy.marble_rad/0.035f);
  shader.setUniform("FRACTAL_ITER", level_copy.FractalIter);
  shader.setUniform("REFL_REFR_ENABLED", Refl_Refr_Enabled);
  shader.setUniform("MARBLE_MODE", MarbleType);
}



//================= Fractal (must match distance_estimators.glsl) =================
namespace {
  struct FracSetup {
    float scale;
    Eigen::Matrix3f R1, R2;
    Eigen::Vector3f shift;
    int iters;
    int type;
    bool mirror[3];
    int radial_global, radial_internal;
  };

  const float FRAC_BAIL2 = 1e8f;
  const float FRAC_WMAX = 1e11f;
  const float FRAC_WMIN = 1e-20f;

  Eigen::Matrix3f FracRotation(float angle, const Eigen::Vector3f& axis, const Eigen::Vector3f& def) {
    Eigen::Vector3f a = axis;
    if (!(a.norm() > 1e-6f)) { a = def; }
    return Eigen::AngleAxisf(-angle, a.normalized()).toRotationMatrix();
  }

  FracSetup MakeFracSetup(const Level& lvl, const FractalParams& fp) {
    FracSetup f;
    f.scale = fp[0];
    f.R1 = FracRotation(fp[1], lvl.rot_axis1, Eigen::Vector3f::UnitZ());
    f.R2 = FracRotation(fp[2], lvl.rot_axis2, Eigen::Vector3f::UnitX());
    f.shift = fp.segment<3>(3);
    f.iters = lvl.FractalIter;
    f.type = lvl.sym_type;
    for (int i = 0; i < 3; i++) f.mirror[i] = lvl.sym_mirror[i];
    f.radial_global = lvl.sym_radial_global;
    f.radial_internal = lvl.sym_radial_internal;
    return f;
  }

  inline void RadialFold(Eigen::Vector4f& p, int n) {
    const float sector = 2.0f * PI / float(n);
    float a = std::atan2(p.z(), p.x()) + 0.5f * sector;
    a = std::abs(a - sector * std::floor(a / sector) - 0.5f * sector);
    const float r = std::sqrt(p.x()*p.x() + p.z()*p.z());
    p.x() = r * std::cos(a);
    p.z() = r * std::sin(a);
  }

  inline void MengerFold(Eigen::Vector4f& p) {
    float a = std::min(p.x() - p.y(), 0.0f);
    p.x() -= a; p.y() += a;
    a = std::min(p.x() - p.z(), 0.0f);
    p.x() -= a; p.z() += a;
    a = std::min(p.y() - p.z(), 0.0f);
    p.y() -= a; p.z() += a;
  }

  inline void FracStep(Eigen::Vector4f& p, const FracSetup& f) {
    if (f.radial_internal > 1) { RadialFold(p, f.radial_internal); }
    for (int k = 0; k < 3; k++) { if (f.mirror[k]) { p[k] = std::abs(p[k]); } }
    p.head<3>() = f.R1 * p.head<3>();
    if (f.type == SYM_MENGER) {
      MengerFold(p);
    } else if (f.type == SYM_SIERPINSKI) {
      float a = std::min(p.x() + p.y(), 0.0f); p.x() -= a; p.y() -= a;
      a = std::min(p.x() + p.z(), 0.0f); p.x() -= a; p.z() -= a;
      a = std::min(p.y() + p.z(), 0.0f); p.y() -= a; p.z() -= a;
    } else if (f.type == SYM_DIAGONAL) {
      const float a = std::min(p.x() - p.y(), 0.0f); p.x() -= a; p.y() += a;
    }
    p.head<3>() = f.R2 * p.head<3>();
    p *= f.scale;
    p.head<3>() += f.shift;
  }

  inline bool FracExit(const Eigen::Vector4f& p) {
    return p.head<3>().squaredNorm() > FRAC_BAIL2 || p.w() > FRAC_WMAX || p.w() < FRAC_WMIN;
  }

  inline float DEBox6(const Eigen::Vector4f& p) {
    const Eigen::Vector3f a = p.segment<3>(0).cwiseAbs() - Eigen::Vector3f(6.0f, 6.0f, 6.0f);
    return (std::min(std::max(std::max(a.x(), a.y()), a.z()), 0.0f) + a.cwiseMax(0.0f).norm()) / p.w();
  }
}

float Scene::DE(const Eigen::Vector3f& pt) const {
  const FracSetup f = MakeFracSetup(level_copy, frac_params_smooth);
  Eigen::Vector4f p;
  p << pt, 1.0f;
  if (f.radial_global > 1) { RadialFold(p, f.radial_global); }
  for (int i = 0; i < f.iters; ++i) {
    FracStep(p, f);
    if (FracExit(p)) { break; }
  }
  return DEBox6(p);
}

//nearest point via the numerical gradient (works for every symmetry type)
Eigen::Vector3f Scene::NPGradient(const Eigen::Vector3f& pt, float& de, Eigen::Vector3f& n) const {
  de = DE(pt);
  const float h = std::max(marble_rad * 1e-3f, 1e-6f);
  Eigen::Vector3f g(
    DE(pt + Eigen::Vector3f(h, 0, 0)) - DE(pt - Eigen::Vector3f(h, 0, 0)),
    DE(pt + Eigen::Vector3f(0, h, 0)) - DE(pt - Eigen::Vector3f(0, h, 0)),
    DE(pt + Eigen::Vector3f(0, 0, h)) - DE(pt - Eigen::Vector3f(0, 0, h)));
  if (!(g.norm() > 1e-12f)) { g = GravityUp(); }
  n = g.normalized();
  return pt - n * de;
}

//Exact nearest point by folding + unfolding (default menger symmetry only)
Eigen::Vector3f Scene::NP(const Eigen::Vector3f& pt) const {
  const FracSetup f = MakeFracSetup(level_copy, frac_params_smooth);
  const float frac_scale = f.scale;
  const Eigen::Vector3f frac_shift = f.shift;

  static std::vector<Eigen::Vector4f, Eigen::aligned_allocator<Eigen::Vector4f>> p_hist;
  p_hist.clear();
  Eigen::Vector4f p;
  p << pt, 1.0f;
  //Fold the point, keeping history
  int done = 0;
  for (int i = 0; i < f.iters; ++i) {
    //absFold
    p_hist.push_back(p);
    p.segment<3>(0) = p.segment<3>(0).cwiseAbs();
    //rotation 1
    p.head<3>() = f.R1 * p.head<3>();
    //mengerFold
    p_hist.push_back(p);
    MengerFold(p);
    //rotation 2
    p.head<3>() = f.R2 * p.head<3>();
    //scaleTrans
    p *= frac_scale;
    p.segment<3>(0) += frac_shift;
    done++;
    if (FracExit(p)) { break; }
  }
  //Get the nearest point
  Eigen::Vector3f n = p.segment<3>(0).cwiseMax(-6.0f).cwiseMin(6.0f);
  //Then unfold the nearest point (reverse order)
  for (int i = 0; i < done; ++i) {
    //scaleTrans
    n.segment<3>(0) -= frac_shift;
    n /= frac_scale;
    //rotation 2 inverse
    n = f.R2.transpose() * n;
    //mengerUnfold
    p = p_hist.back(); p_hist.pop_back();
    const float mx = std::max(p[0], p[1]);
    if (std::min(p[0], p[1]) < std::min(mx, p[2])) {
      std::swap(n[1], n[2]);
    }
    if (mx < p[2]) {
      std::swap(n[0], n[2]);
    }
    if (p[0] < p[1]) {
      std::swap(n[0], n[1]);
    }
    //rotation 1 inverse
    n = f.R1.transpose() * n;
    //absUnfold
    p = p_hist.back(); p_hist.pop_back();
    if (p[0] < 0.0f) {
      n[0] = -n[0];
    }
    if (p[1] < 0.0f) {
      n[1] = -n[1];
    }
    if (p[2] < 0.0f) {
      n[2] = -n[2];
    }
  }
  return n;
}

bool Scene::MarbleCollision(float& delta_v) {
  //Check if the distance estimate indicates a collision
  const float de = DE(marble_pos);
  const float gr = (level_copy.ground_ratio > 1.0f) ? level_copy.ground_ratio : ground_ratio;
  if (de >= marble_rad) {
    return de < marble_rad * gr;
  }

  const bool rubber = level_copy.rubber_ball;
  const bool deep = de < marble_rad * 0.001f;

  //Check if the marble has been crushed by the fractal (a rubber ball survives)
  if (deep && !rubber) {
    if (!SETTINGS.stg.crush_effects_off && !level_editor) {
      float d0;
      Eigen::Vector3f n;
      NPGradient(marble_pos, d0, n);
      StartCrush(-n);
    } else {
      sound_shatter.play();
      marble_dead = true;
    }
    return false;
  }

  //Find the nearest point and the collision direction (marble -> surface)
  Eigen::Vector3f np, dn;
  bool use_gradient = deep || !level_copy.HasDefaultSymmetry();
  if (!use_gradient) {
    np = NP(marble_pos);
    const Eigen::Vector3f d = np - marble_pos;
    if (d.norm() > 1e-9f) {
      dn = d.normalized();
    } else {
      use_gradient = true;
    }
  }
  if (use_gradient) {
    float d0;
    Eigen::Vector3f n;
    np = NPGradient(marble_pos, d0, n);
    //the gradient points out of the surface, the collision direction points into it
    dn = -n;
  }

  //Apply the offset to the marble's position and velocity
  const float dv = marble_vel.dot(dn);
  delta_v = std::max(delta_v, dv);
  marble_pos = np - dn * marble_rad;
  if (rubber) {
    if (dv > 0.0f) {
      const float weight = std::max(level_copy.ball_weight, 0.01f);
      float e = std::min(std::max(level_copy.rubber_bounce * std::pow(weight, -0.25f), 0.0f), 0.98f);
      if (dv < marble_rad * 0.01f) { e = 0.2f; } //let it come to rest
      marble_vel -= dn * (dv * (1.0f + e));
      //squash on impact, heavier balls deform more
      squash_vel += std::min(dv / marble_rad, 1.5f) * 0.6f * std::sqrt(weight);
      squash_dir = dn;
    }
  } else {
    marble_vel -= dn * (dv * marble_bounce);
  }
  return true;
}

void Scene::EditorPlace(int mousex, int mousey)
{
	switch (cur_ed_mode)
	{
	case PLACE_MARBLE:
		level_copy.start_pos = MouseRayCast(mousex, mousey, level_copy.marble_rad);
		break;
	case PLACE_FLAG:
		level_copy.end_pos = MouseRayCast(mousex, mousey);
		break;
	case PLACE_EXTRA_FLAG:
		if (ed_place_index >= 0 && ed_place_index < level_copy.num_flags)
			level_copy.flags[ed_place_index] = MouseRayCast(mousex, mousey);
		break;
	case PLACE_CHECKPOINT:
		if (ed_place_index >= 0 && ed_place_index < level_copy.num_checkpoints)
			level_copy.checkpoints[ed_place_index] = MouseRayCast(mousex, mousey);
		break;
	default:
		break;
	}
}

Eigen::Vector3f Scene::RenderMarblePos() const
{
	//no interpolation for teleports (respawn, level start)
	if ((marble_pos - prev_marble_pos).norm() > 20.0f * marble_rad) return marble_pos;
	return prev_marble_pos + (marble_pos - prev_marble_pos) * render_alpha;
}

void Scene::WriteRenderer(Renderer & rd)
{
	//Interpolate the camera between the last two simulation ticks
	Eigen::Matrix4f m = cam_mat;
	const Eigen::Vector3f p0 = prev_cam_mat.block<3, 1>(0, 3);
	const Eigen::Vector3f p1 = cam_mat.block<3, 1>(0, 3);
	if (render_alpha < 1.0f && (p1 - p0).norm() < 1.0f)
	{
		const Eigen::Matrix3f r0 = prev_cam_mat.block<3, 3>(0, 0);
		const Eigen::Matrix3f r1 = cam_mat.block<3, 3>(0, 0);
		const Eigen::Quaternionf q0(r0), q1(r1);
		m.block<3, 3>(0, 0) = q0.slerp(render_alpha, q1).toRotationMatrix();
		m.block<3, 1>(0, 3) = p0 + (p1 - p0) * render_alpha;
	}
	//Update the camera
	vec3 cam_pos = vec3(m(0, 3), m(1, 3), m(2, 3));
	vec3 dirx = vec3(m(0, 0), m(1, 0), m(2, 0));
	vec3 diry = vec3(m(0, 1), m(1, 1), m(2, 1));
	vec3 dirz = -vec3(m(0, 2), m(1, 2), m(2, 2));
	rd.camera.SetPosition(cam_pos);
	rd.camera.SetDirX(dirx);
	rd.camera.SetDirY(diry);
	rd.camera.SetDirZ(dirz);
	rd.camera.SetCameraSize(camera_size*level_copy.marble_rad / 0.035f);
	rd.camera.eye_separation = SETTINGS.stg.eye_separation*level_copy.marble_rad / 0.035f;
	//write all the uniform values to the rendering pipeline
	for (auto &shader : rd.shader_pipeline)
	{
		WriteShader(shader);
	}
}

void Scene::WriteShader(ComputeShader& shader)
{

	if (level_editor)
	{
		shader.setUniform("iMarblePos", vec3(level_copy.start_pos.x(), level_copy.start_pos.y(), level_copy.start_pos.z()));
		shader.setUniform("iFlagPos", vec3(level_copy.end_pos.x(), level_copy.end_pos.y(), level_copy.end_pos.z()));
	}
	else
	{
		const Eigen::Vector3f rmp = RenderMarblePos();
		shader.setUniform("iMarblePos", free_camera ?
			vec3(999.0f, 999.0f, 999.0f) :
			vec3(rmp.x(), rmp.y(), rmp.z())
		);
		shader.setUniform("iFlagPos", free_camera ?
			vec3(-999.0f, -999.0f, -999.0f) :
			vec3(flag_pos.x(), flag_pos.y(), flag_pos.z())
		);
	}

	if (cam_mode != INTRO)
	{
		shader.setUniform("LIGHT_DIRECTION", vec3(level_copy.light_dir[0], level_copy.light_dir[1], level_copy.light_dir[2]));
		shader.setUniform("PBR_ENABLED", PBR_Enabled);
		shader.setUniform("PBR_METALLIC", level_copy.PBR_metal);
		shader.setUniform("PBR_ROUGHNESS", level_copy.PBR_roughness);
	}
	else
	{
		shader.setUniform("LIGHT_DIRECTION", vec3(LIGHT_DIRECTION[0], LIGHT_DIRECTION[1], LIGHT_DIRECTION[2]));
		shader.setUniform("PBR_ENABLED", PBR_Enabled);
		shader.setUniform("PBR_METALLIC", PBR_METALLIC);
		shader.setUniform("PBR_ROUGHNESS", PBR_ROUGHNESS);
	}

	shader.setUniform("BACKGROUND_COLOR", vec3(level_copy.background_col[0], level_copy.background_col[1], level_copy.background_col[2]));
	shader.setUniform("LIGHT_COLOR", vec3(level_copy.light_col[0], level_copy.light_col[1], level_copy.light_col[2]));

	shader.setUniform("iMarbleRad", level_copy.marble_rad);

	shader.setUniform("iFracScale", frac_params_smooth[0]);
	shader.setUniform("iFracAng1", frac_params_smooth[1]);
	shader.setUniform("iFracAng2", frac_params_smooth[2]);
	shader.setUniform("iFracShift", vec3(frac_params_smooth[3], frac_params_smooth[4], frac_params_smooth[5]));
	shader.setUniform("iFracCol", vec3(frac_params_smooth[6], frac_params_smooth[7], frac_params_smooth[8]));

	shader.setUniform("SHADOWS_ENABLED", Shadows_Enabled);
	shader.setUniform("FOG_ENABLED", Fog_Enabled);
	shader.setUniform("FRACTAL_ITER", level_copy.FractalIter);
	shader.setUniform("REFL_REFR_ENABLED", Refl_Refr_Enabled);
	shader.setUniform("FLAG_GLOW", SETTINGS.stg.flag_glow);

	shader.setUniform("gamma_material", gamma_material);
	shader.setUniform("gamma_sky", gamma_sky);
	shader.setUniform("gamma_camera", gamma_camera);

	shader.setUniform("time", time);

	//===== Rebuild extensions =====
	const bool intro = (cam_mode == INTRO || cam_mode == SCREEN_SAVER);
	const Level& L = intro ? default_level : level_copy;

	//fractal rotation axes
	{
		const FracSetup f = MakeFracSetup(L, frac_params_smooth);
		shader.setUniform("iFracRot1", glm::make_mat3(f.R1.data()), false);
		shader.setUniform("iFracRot2", glm::make_mat3(f.R2.data()), false);
	}

	//symmetry
	shader.setUniform("SYM_DEFAULT", (int)L.HasDefaultSymmetry());
	shader.setUniform("SYM_TYPE", L.sym_type);
	shader.setUniform("SYM_MIRROR", vec3(L.sym_mirror[0] ? 1.f : 0.f, L.sym_mirror[1] ? 1.f : 0.f, L.sym_mirror[2] ? 1.f : 0.f));
	shader.setUniform("SYM_RADIAL_GLOBAL", L.sym_radial_global);
	shader.setUniform("SYM_RADIAL_INTERNAL", L.sym_radial_internal);

	//glow
	shader.setUniform("FRACTAL_GLOW", (int)(SETTINGS.stg.fractal_glow || L.glow_enabled));
	shader.setUniform("GLOW_CUSTOM", (int)L.glow_enabled);
	shader.setUniform("GLOW_COLOR", vec3(L.glow_color[0], L.glow_color[1], L.glow_color[2]));
	shader.setUniform("GLOW_PATTERN", vec3(L.glow_pattern[0], L.glow_pattern[1], L.glow_pattern[2]));
	shader.setUniform("GLOW_BRIGHTNESS", L.glow_brightness);
	float glow_beat = 1.0f;
	if (L.glow_beat_sync)
	{
		const float s = std::min(std::max(L.glow_beat_strength, 0.0f), 1.0f);
		glow_beat = (1.0f - s) + s * (0.1f + 2.2f * music_beat);
	}
	shader.setUniform("GLOW_BEAT", glow_beat);

	//sky
	shader.setUniform("CUSTOM_SKY", (int)L.custom_sky);
	shader.setUniform("SKY_COLOR", vec3(L.sky_color[0], L.sky_color[1], L.sky_color[2]));

	//marble type + rubber deformation
	const bool rubber = !intro && level_copy.rubber_ball;
	const bool crushing = !intro && crush_timer > 0;
	const bool dumpling = crushing && crush_metal;
	shader.setUniform("MARBLE_MODE", rubber ? 3 : (dumpling ? 4 : MarbleType));
	shader.setUniform("iMarbleSquash", glm::vec4(squash_dir.x(), squash_dir.y(), squash_dir.z(), (rubber || dumpling) ? squash : 0.0f));
	shader.setUniform("iMarbleLumps", dumpling ? 1.0f : 0.0f);
	if (crushing && !crush_metal)
	{
		//the marble itself is gone, only the shards remain
		shader.setUniform("iMarblePos", vec3(999.0f, 999.0f, 999.0f));
	}

	//glass shards
	const int ns = (crushing && !crush_metal) ? MAX_SHARDS : 0;
	shader.setUniform("iShardCount", ns);
	if (ns > 0)
	{
		float sp[MAX_SHARDS * 4], sq[MAX_SHARDS * 4];
		Eigen::Vector3f bmin = shard_pos[0], bmax = shard_pos[0];
		float smax = 0.f;
		for (int i = 0; i < ns; i++)
		{
			sp[4 * i] = shard_pos[i].x(); sp[4 * i + 1] = shard_pos[i].y(); sp[4 * i + 2] = shard_pos[i].z();
			sp[4 * i + 3] = shard_size[i];
			const float h = 0.5f * shard_angle[i];
			sq[4 * i] = shard_axis[i].x() * std::sin(h); sq[4 * i + 1] = shard_axis[i].y() * std::sin(h);
			sq[4 * i + 2] = shard_axis[i].z() * std::sin(h); sq[4 * i + 3] = std::cos(h);
			bmin = bmin.cwiseMin(shard_pos[i]); bmax = bmax.cwiseMax(shard_pos[i]);
			smax = std::max(smax, shard_size[i]);
		}
		const Eigen::Vector3f c = 0.5f * (bmin + bmax);
		shader.setUniform("iShardBound", glm::vec4(c.x(), c.y(), c.z(), 0.5f * (bmax - bmin).norm() + smax));
		shader.setUniformArray4("iShardPos", sp, ns);
		shader.setUniformArray4("iShardRot", sq, ns);
		shader.setUniform("iShardColor", level_copy.rubber_ball ? vec3(0.03f, 0.03f, 0.03f) :
			(MarbleType == 2 ? vec3(0.85f, 0.82f, 0.78f) : vec3(0.75f, 0.88f, 0.95f)));
	}

	//flags are flipped for planets and inverted gravity
	shader.setUniform("iFlagScale", FlagsFlipped() ? -level_copy.marble_rad : level_copy.marble_rad);

	//multiple flags + checkpoints
	const bool objects_visible = level_editor || (!free_camera && !intro && flag_pos.x() < 998.0f);
	const int nf = objects_visible ? level_copy.num_flags : 0;
	const int nc = objects_visible ? level_copy.num_checkpoints : 0;
	const bool multi = IsMultiFlag() && !level_editor;
	//ordered checkpoints: flags are locked until all checkpoints are hit
	const bool flags_locked = !level_editor && CheckpointsPending();
	const int next_cp = GetNextCheckpoint();
	float flag_state[MAX_EXTRA_FLAGS], cp_state[MAX_CHECKPOINTS];
	for (int i = 0; i < MAX_EXTRA_FLAGS; i++)
	{
		//sequence index of extra flag i is i+1 (the main flag is flag 1)
		flag_state[i] = multi ? (i + 1 < next_flag ? 2.f : (i + 1 == next_flag ? 1.f : 0.f)) : 1.f;
		if (flags_locked && flag_state[i] < 1.5f) flag_state[i] = 0.f;
	}
	for (int i = 0; i < MAX_CHECKPOINTS; i++)
	{
		//0 inactive, 1 hit, 2 next target (ordered mode)
		cp_state[i] = (level_editor || cp_active[i]) ? 1.f : 0.f;
		if (!level_editor && level_copy.ordered_checkpoints && i == next_cp) cp_state[i] = 2.f;
	}
	shader.setUniform("iGoalState", flags_locked ? 0.f : (multi ? (next_flag == 0 ? 1.f : 2.f) : 1.f));
	shader.setUniform("iFlagCount", nf);
	shader.setUniform("iCPCount", nc);
	if (nf > 0)
	{
		shader.setUniformArray3("iFlagArr", level_copy.flags[0].data(), nf);
		shader.setUniformArray1("iFlagState", flag_state, nf);
	}
	if (nc > 0)
	{
		shader.setUniformArray3("iCPArr", level_copy.checkpoints[0].data(), nc);
		shader.setUniformArray1("iCPState", cp_state, nc);
	}
}


void Scene::Cheat_ColorChange() {
  if (!enable_cheats) { return; }
  level_copy.params[6] = frac_params_smooth[6] = frac_params[6] = float((rand() % 201) - 100) * 0.01f;
  level_copy.params[7] = frac_params_smooth[7] = frac_params[7] = float((rand() % 201) - 100) * 0.01f;
  level_copy.params[8] = frac_params_smooth[8] = frac_params[8] = float((rand() % 201) - 100) * 0.01f;
}
void Scene::Cheat_FreeCamera() {
  if (!enable_cheats) { return; }
  free_camera = !free_camera;
}
void Scene::Cheat_Gravity() {
  if (!enable_cheats) { return; }
  gravity_type = (gravity_type + 1) % 3;
}
void Scene::Cheat_HyperSpeed() {
  if (!enable_cheats) { return; }
  hyper_speed = !hyper_speed;
}
void Scene::Cheat_IgnoreGoal() {
  if (!enable_cheats) { return; }
  ignore_goal = !ignore_goal;
}
void Scene::Cheat_Motion() {
  if (!enable_cheats) { return; }
  disable_motion = !disable_motion;
}
void Scene::Cheat_Planet() {
  if (!enable_cheats) { return; }
  level_copy.planet = !level_copy.planet;
}
void Scene::Cheat_Zoom() {
  if (!enable_cheats) { return; }
  zoom_to_scale = !zoom_to_scale;
}
void Scene::Cheat_Param(int param) {
  if (!enable_cheats) { return; }
  param_mod = param;
}

void Scene::ExitEditor()
{
	levels.StopAllMusic();
	level_editor = false;
}


//inverse of MouseRayCast / get_ray(): world position => window pixel
bool Scene::ProjectToScreen(const Eigen::Vector3f& p, float& sx, float& sy) const
{
	const Eigen::Vector3f dirx = cam_mat.block<3, 1>(0, 0);
	const Eigen::Vector3f diry = cam_mat.block<3, 1>(0, 1);
	const Eigen::Vector3f dirz = -cam_mat.block<3, 1>(0, 2);
	const Eigen::Vector3f rel = p - cam_mat.block<3, 1>(0, 3);
	const float z = rel.dot(dirz);
	if (z <= 1e-7f) return false; //behind the camera
	const float t = std::tan(SETTINGS.stg.FOV * PI / 360.0f);
	const float aspect = (float)WinX / (float)std::max(WinY, 1);
	const float shift_x = rel.dot(dirx) / z;
	const float shift_y = rel.dot(diry) / z;
	sx = 0.5f * (shift_x / (t * aspect) + 1.0f) * WinX;
	sy = 0.5f * (1.0f - shift_y / t) * WinY;
	return true;
}

std::vector<Scene::EditorMarker> Scene::GetEditorMarkers() const
{
	std::vector<EditorMarker> m;
	const float r = level_copy.marble_rad;
	auto up_at = [&](const Eigen::Vector3f& p) -> Eigen::Vector3f {
		Eigen::Vector3f up = level_copy.planet ? p.normalized() : Eigen::Vector3f(0.f, 1.f, 0.f);
		if (FlagsFlipped()) up = -up;
		return up;
	};
	const bool multi = level_copy.multi_flag && level_copy.num_flags > 0;

	m.push_back({ level_copy.start_pos + up_at(level_copy.start_pos) * (1.6f * r), "Start",
		sf::Color(255, 255, 255), cur_ed_mode == PLACE_MARBLE });
	m.push_back({ level_copy.end_pos + up_at(level_copy.end_pos) * (5.4f * r), multi ? "F1" : "Goal",
		sf::Color(255, 220, 60), cur_ed_mode == PLACE_FLAG });
	for (int i = 0; i < level_copy.num_flags; i++)
	{
		m.push_back({ level_copy.flags[i] + up_at(level_copy.flags[i]) * (5.4f * r),
			multi ? "F" + std::to_string(i + 2) : "Goal " + std::to_string(i + 2),
			sf::Color(255, 150, 40), cur_ed_mode == PLACE_EXTRA_FLAG && ed_place_index == i });
	}
	for (int i = 0; i < level_copy.num_checkpoints; i++)
	{
		m.push_back({ level_copy.checkpoints[i] + up_at(level_copy.checkpoints[i]) * (6.8f * r),
			"C" + std::to_string(i + 1),
			sf::Color(80, 220, 255), cur_ed_mode == PLACE_CHECKPOINT && ed_place_index == i });
	}
	return m;
}

Eigen::Vector3f Scene::MouseRayCast(int mousex, int mousey, float min_dist)
{
	//exactly the same ray as get_ray() in camera.glsl, so the object lands under the cursor
	const float sx = (float)mousex / (float)std::max(WinX, 1);
	const float sy = (float)mousey / (float)std::max(WinY, 1);
	const float t = std::tan(SETTINGS.stg.FOV * PI / 360.0f);
	const float aspect = (float)WinX / (float)std::max(WinY, 1);
	const float shift_x = t * (2.0f * sx - 1.0f) * aspect;
	const float shift_y = -t * (2.0f * sy - 1.0f);

	const Eigen::Vector3f dirx = cam_mat.block<3, 1>(0, 0);
	const Eigen::Vector3f diry = cam_mat.block<3, 1>(0, 1);
	const Eigen::Vector3f dirz = -cam_mat.block<3, 1>(0, 2);
	const Eigen::Vector3f pos = cam_mat.block<3, 1>(0, 3);
	const float size = camera_size * level_copy.marble_rad / 0.035f;

	const Eigen::Vector3f origin = pos + size * (dirx * shift_x + diry * shift_y);
	const Eigen::Vector3f ray = (dirx * shift_x + diry * shift_y + dirz).normalized();

	//the stop distance scales with the marble, important for tiny marbles
	const float md = std::min(min_dist, level_copy.marble_rad * 0.05f);
	return RayMarch(origin, ray, std::max(md, 1e-7f));
}

Eigen::Vector3f Scene::RayMarch(const Eigen::Vector3f & pt, const Eigen::Vector3f & ray, float min_dist)
{
	float td = 0;
	for (int i = 0; i < MAX_MARCHES && td < MAX_DIST; i++)
	{
		float de = DE(pt + td * ray);
		if (de < min_dist)
		{
			break;
		}
		td += de;
	}
	return pt + td * ray;
}
