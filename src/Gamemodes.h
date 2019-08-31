#pragma once

#include<Overlays.h>
#include<Scene.h>
#include<Localization.h>
#include<Interface.h>

#include <AntTweakBar.h>

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
	MIDPOINT
};




//Global variables
extern sf::Vector2i mouse_pos, mouse_prev_pos;
extern bool all_keys[sf::Keyboard::KeyCount];
extern bool mouse_clicked;
extern bool show_cheats;
extern InputState io_state;

//Constants
extern float target_fps;

extern GameMode game_mode;

void OpenMainMenu(Scene * scene, Overlays * overlays);

void OpenEditor(Scene * scene, Overlays * overlays, int level);
void PlayLevel(Scene * scene, sf::RenderWindow * window, int level);

void OpenControlMenu(Scene * scene, Overlays * overlays);

void OpenPauseMenu(Scene * scene, Overlays * overlays);

void OpenScreenSaver(Scene * scene, Overlays * overlays);

void PlayNewGame(Scene * scene, sf::RenderWindow * window, int level);

void OpenTestWindow();

void OpenLevelMenu(Scene* scene, Overlays* overlays);
void ConfirmLevelDeletion(int lvl, Scene* scene, Overlays* overlays);

void ConfirmEditorExit(Scene * scene, Overlays * overlays);
void LockMouse(sf::RenderWindow& window);
void UnlockMouse(sf::RenderWindow& window);
void PauseGame(sf::RenderWindow& window, Overlays * overlays, Scene * scene);
int DirExists(const char *path);

void FirstStart(Overlays* overlays);

void SetPointers(sf::RenderWindow *w, Scene* scene, Overlays* overlays, Renderer* rd, sf::RenderTexture *render, sf::RenderTexture *screenshot, sf::Texture *main, sf::Texture *screensht, sf::RectangleShape *rmain, sf::RectangleShape *rscr, sf::Shader *shader);
void TakeScreenshot();

void TW_CALL ApplySettings(void * data);

void InitializeATBWindows(float * fps, float * target_fps);

template < typename T > std::string num2str(const T& n)
{
	std::ostringstream stm;
	if (n < 10) stm << "0";
	stm << n;
	return stm.str();
}


class Sounds
{

};
