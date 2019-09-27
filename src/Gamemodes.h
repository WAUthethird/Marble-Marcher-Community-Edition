#pragma once

#include<Overlays.h>
#include<Scene.h>
#include<Localization.h>
#include<Interface.h>

#include <AntTweakBar.h>

#define DEBUG_MODE 1

enum GameMode {
	FIRST_START,
	MAIN_MENU,
	PLAYING,
	PAUSED,
	SCREEN_SAVER,
	CONTROLS,
	LEVELS,
	LEVEL_EDITOR,
	CREDITS,
	MIDPOINT,
	ABOUT,
	MENU
};


struct GamepadState
{
	float axis_value[sf::Joystick::AxisCount] = { 0.f };
	bool  buttons[sf::Joystick::ButtonCount] = { false };
};

struct InputRecord
{
	float move_x, move_y;
	float view_x, view_y;
	float cam_z;
	bool mouse_clicked;
};


//Global variables
extern bool recording;
extern bool replay;

extern sf::Vector2i mouse_pos, mouse_prev_pos;
extern bool all_keys[sf::Keyboard::KeyCount];
extern bool mouse_clicked;
extern bool show_cheats;
extern InputState io_state;
extern GamepadState gamepad_state;

//Constants
extern float target_fps;

extern GameMode game_mode;

void OpenMainMenu(Scene * scene, Overlays * overlays);

void OpenCredits(Scene * scene, Overlays * overlays);

void OpenEditor(Scene * scene, Overlays * overlays, int level);
void PlayLevel(Scene * scene, sf::RenderWindow * window, int level);

void OpenControlMenu(Scene * scene, Overlays * overlays);

void ResumeGame(sf::RenderWindow &window);

void OpenPauseMenu(Scene * scene, Overlays * overlays);

void OpenScreenSaver(Scene * scene, Overlays * overlays);

void PlayNewGame(Scene * scene, sf::RenderWindow * window, int level);

void OpenTestWindow();

void OpenLevelMenu(Scene* scene, Overlays* overlays);
void ConfirmLevelDeletion(int lvl, Scene* scene, Overlays* overlays);

void ConfirmEditorExit(Scene * scene, Overlays * overlays);
void DisplayError(std::string error_text);
void LockMouse(sf::RenderWindow& window);
void UnlockMouse(sf::RenderWindow& window);
void PauseGame(sf::RenderWindow& window, Overlays * overlays, Scene * scene);
int DirExists(const char *path);

void FirstStart(Overlays* overlays);

void SetPointers(sf::RenderWindow * w, Scene * scene, Overlays * overlays, Renderer * rd, sf::Texture * main, sf::Texture * screensht);
void TakeScreenshot();

void TW_CALL ApplySettings(void * data);

void SaveRecord(float mx, float my, float vx, float vy, float cz, bool mc);

InputRecord GetRecord();

void ApplyButton(int NUM, int MODE);

void InitializeATBWindows(float * fps, float * target_fps);




