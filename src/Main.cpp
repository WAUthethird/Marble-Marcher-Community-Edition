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

#include <Gamemodes.h>
#include "Level.h"
#include "Res.h"
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <AntTweakBar.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>





#ifdef _WIN32
#include <Windows.h>
#define ERROR_MSG(x) MessageBox(nullptr, TEXT(x), TEXT("ERROR"), MB_OK);
#else
#define ERROR_MSG(x) std::cerr << x << std::endl;
#endif

//Graphics settings
static bool VSYNC = true;


#if defined(_WIN32)
int WinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR lpCmdLine, int nCmdShow) {
#else
int main(int argc, char *argv[]) {
#endif
	//Load the music
	sf::Music menu_music;
	menu_music.openFromFile(menu_ogg);
	menu_music.setLoop(true);
	sf::Music level_music[num_level_music];
	level_music[0].openFromFile(level1_ogg);
	level_music[0].setLoop(true);
	level_music[1].openFromFile(level2_ogg);
	level_music[1].setLoop(true);
	level_music[2].openFromFile(level3_ogg);
	level_music[2].setLoop(true);
	level_music[3].openFromFile(level4_ogg);
	level_music[3].setLoop(true);
	sf::Music credits_music;
	credits_music.openFromFile(credits_ogg);
	credits_music.setLoop(true);

	bool first_start = SETTINGS.Load(settings_bin);

	//all of the fonts
	Fonts fonts;
	LOCAL.LoadLocalsFromFolder(local_folder, &fonts);
	
	//all declarations
	sf::RenderWindow window;
  
	Renderer rend(main_config);
	sf::Texture main_txt, screenshot_txt;
	
	//Create the fractal scene
	Scene scene(level_music);
	//Create the old menus
	Overlays overlays(&scene);
	sf::Clock clock;
	float smooth_fps = target_fps;
	float lag_ms = 0.0f;
	mouse_pos = sf::Vector2i(0, 0);
	mouse_prev_pos = sf::Vector2i(0, 0);
  
	SetPointers(&window, &scene, &overlays, &rend, &main_txt, &screenshot_txt);

	ApplySettings(nullptr);

	scene.levels.LoadLevelsFromFolder(level_folder);
	scene.levels.LoadMusicFromFolder(music_folder);

	InitializeATBWindows(&smooth_fps, &target_fps);

	window.requestFocus();
	UpdateAspectRatio(window.getSize().x, window.getSize().y);
	//set window icon
	sf::Image icon;
	icon.loadFromFile(icon_png); 
	window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

	//force fullscreen mode
	bool fullscreen = true;

	rend.LoadExternalTextures(textures_folder);
  
	sf::View default_window_view = window.getDefaultView();
 
	scene.SetWindowResolution(window.getSize().x, window.getSize().y);

	//temporary level generation code
	/*for (int i = 0; i < 24; i++)
	{
		all_levels[i].desc = "Official Level by Codeparade";
		all_levels[i].SaveToFile(std::string(level_folder) + "/" + ConvertSpaces2_(all_levels[i].txt)+".lvl", i, (i<24)?(i+1):-1);
	}*/

	overlays.SetScale(float(window.getSize().x) / 1280.0f);

	scene.StartDefault();

	io_state.window_size = sf::Vector2f(window.getSize().x, window.getSize().y);
	float prev_s = 0;

	
	if (first_start)
	{
		FirstStart(&overlays);
	}
	else
	{
		OpenMainMenu(&scene, &overlays);
	}

	//Main loop
	while (window.isOpen())
	{
		sf::Event event;
		window.clear(sf::Color::White);
		float mouse_wheel = 0.0f;
		mouse_prev_pos = mouse_pos;
		io_state.mouse_prev = sf::Vector2f(mouse_prev_pos.x, mouse_prev_pos.y);
		io_state.wheel = mouse_wheel;
		io_state.mouse_press[2] = false;
		io_state.mouse_press[0] = false;
		while (window.pollEvent(event)) 
		{
			bool handled = overlays.TwManageEvent(&event);

			for (int i = 0; i < sf::Keyboard::KeyCount; i++)
			{
				io_state.key_press[i] = false;
			}

			if (event.type == sf::Event::Closed) 
			{
				window.close();
				break;
			}

			else if (event.type == sf::Event::LostFocus) 
			{
				if (game_mode == PLAYING) 
				{
					PauseGame(window, &overlays, &scene);
				}
			}
			else if (event.type == sf::Event::Resized) 
			{
				overlays.SetScale( std::max(float(event.size.width), float(event.size.height))/ 1280.0f);
				sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
				default_window_view = sf::View(visibleArea);
				window.setView(default_window_view);
				io_state.window_size = sf::Vector2f(window.getSize().x, window.getSize().y);
				UpdateAspectRatio(window.getSize().x, window.getSize().y);
				scene.SetWindowResolution(window.getSize().x, window.getSize().y);
				rend.camera.SetAspectRatio((float)window.getSize().x / (float)window.getSize().y);
			}

			// If event has not been handled by AntTweakBar, process it
			if (!handled)
			{
				if (event.type == sf::Event::KeyPressed) 
				{
					const sf::Keyboard::Key keycode = event.key.code;
					all_keys[keycode] = true;
					io_state.keys[keycode] = true;
					io_state.key_press[keycode] = true;  
					if (event.key.code < 0 || event.key.code >= sf::Keyboard::KeyCount) { continue; }
					if (game_mode == CREDITS)
					{
						OpenMainMenu(&scene, &overlays);
						UnlockMouse(window);
						credits_music.stop();
						scene.levels.StopAllMusic();
					}
					else if (game_mode == MIDPOINT) 
					{
						game_mode = PLAYING;
						scene.SetExposure(1.0f);
						credits_music.stop();
						scene.StartNextLevel();
					}
					else if (keycode == sf::Keyboard::Escape)
					{
						if (game_mode == MAIN_MENU)
						{
							window.close();
							break;
						}
						else if (game_mode == CONTROLS || game_mode == LEVELS) 
						{
							OpenMainMenu(&scene, &overlays);
						}
						else if (game_mode == SCREEN_SAVER) 
						{
							OpenMainMenu(&scene, &overlays);
						}
						else if (game_mode == PAUSED)
						{
							ResumeGame(window);
						}
						else if (game_mode == PLAYING) 
						{
							PauseGame(window, &overlays, &scene);
						}
						else if (game_mode == LEVEL_EDITOR)
						{
							//if no interface objects created
							if (NoObjects())
							{
								ConfirmEditorExit(&scene, &overlays);
							}
							else if(get_glob_obj(focused).action_time < 0.f)//remove confirm window
							{
								RemoveGlobalObject(focused);
							}
						}
					}
					else if (keycode == sf::Keyboard::R) 
					{
						if (game_mode == PLAYING) {
							scene.ResetLevel();
						}
					}
					else if (keycode == sf::Keyboard::F1) 
					{
						if (game_mode == PLAYING) {
							show_cheats = !show_cheats;
							scene.EnbaleCheats();
						}
					}
					else if (keycode == sf::Keyboard::F5) 
					{ 
						TakeScreenshot();
					} 
					else if (keycode == sf::Keyboard::F4)
					{
						overlays.TWBAR_ENABLED = !overlays.TWBAR_ENABLED;
					} 
					else if (keycode == sf::Keyboard::C) 
					{
						scene.Cheat_ColorChange();
					} 	
					else if (keycode == sf::Keyboard::F) 
					{
						scene.Cheat_FreeCamera();
					}
					else if (keycode == sf::Keyboard::G) 
					{
						scene.Cheat_Gravity();
					}
					else if (keycode == sf::Keyboard::H) 
					{
						scene.Cheat_HyperSpeed();
					}
					else if (keycode == sf::Keyboard::I) 
					{
						scene.Cheat_IgnoreGoal();
					}
					else if (keycode == sf::Keyboard::M) 
					{
						scene.Cheat_Motion();
					}
					else if (keycode == sf::Keyboard::P) 
					{
						scene.Cheat_Planet();
					}
					else if (keycode == sf::Keyboard::Z) 
					{
						if (scene.GetParamMod() == -1) 
						{
							scene.Cheat_Zoom();
						}
						else 
						{
							scene.Cheat_Param(-1);
						}
					} 
					if (keycode >= sf::Keyboard::Num0 && keycode <= sf::Keyboard::Num9)
					{
						scene.Cheat_Param(int(keycode) - int(sf::Keyboard::Num1));
					}
				
				}
				else if (event.type == sf::Event::KeyReleased) 
				{
					const sf::Keyboard::Key keycode = event.key.code;
					if (event.key.code < 0 || event.key.code >= sf::Keyboard::KeyCount) { continue; }
					all_keys[keycode] = false;
					io_state.keys[keycode] = false;
				}
				else if (event.type == sf::Event::MouseButtonPressed)
			    {
					if (event.mouseButton.button == sf::Mouse::Left) 
					{
						mouse_pos = sf::Vector2i(event.mouseButton.x, event.mouseButton.y);
						mouse_clicked = true;
						io_state.mouse[0] = true;
						io_state.mouse_press[0] = true;
						if (game_mode == CONTROLS) {
							const Overlays::Texts selected = overlays.GetOption(Overlays::BACK, Overlays::BACK);
							if (selected == Overlays::BACK) {
								OpenMainMenu(&scene, &overlays);
							}
						}
						else if (game_mode == SCREEN_SAVER) {
							OpenMainMenu(&scene, &overlays);
						}
						if (game_mode == LEVEL_EDITOR)
						{
							//stop placing flag/marble
							if (scene.cur_ed_mode == Scene::EditorMode::PLACE_MARBLE
								|| scene.cur_ed_mode == Scene::EditorMode::PLACE_FLAG)
							{
								scene.cur_ed_mode = Scene::EditorMode::DEFAULT;
							}
						}
					}
					else if (event.mouseButton.button == sf::Mouse::Right)
					{
						io_state.mouse[2] = true; 
						io_state.mouse_press[2] = true;
						if (game_mode == PLAYING) {
							scene.ResetLevel();
						}
					}
				}
				else if (event.type == sf::Event::MouseButtonReleased)
			    {
					if (event.mouseButton.button == sf::Mouse::Left) 
					{
						io_state.mouse[0] = false;
						mouse_pos = sf::Vector2i(event.mouseButton.x, event.mouseButton.y);
						mouse_clicked = false;
					}
					else if (event.mouseButton.button == sf::Mouse::Right) {
						io_state.mouse[2] = false;
					}
				}
				else if (event.type == sf::Event::MouseMoved) 
				{
					mouse_pos = sf::Vector2i(event.mouseMove.x, event.mouseMove.y);
					io_state.mouse_pos = sf::Vector2f(mouse_pos.x, mouse_pos.y);
					if (scene.cur_ed_mode == Scene::EditorMode::PLACE_MARBLE)
					{
						Eigen::Vector3f marble_pos = scene.MouseRayCast(mouse_pos.x, mouse_pos.y, scene.level_copy.marble_rad);
						scene.level_copy.start_pos = marble_pos;
					}
					else if (scene.cur_ed_mode == Scene::EditorMode::PLACE_FLAG)
					{
						Eigen::Vector3f flag_pos = scene.MouseRayCast(mouse_pos.x, mouse_pos.y);
						scene.level_copy.end_pos = flag_pos;
					}
				}
				else if (event.type == sf::Event::MouseWheelScrolled)
			    {
					mouse_wheel += event.mouseWheelScroll.delta;
					io_state.wheel = mouse_wheel;
				}
			}
		}
		

		//Check if the game was beat
		if (scene.GetMode() == Scene::FINAL && game_mode != CREDITS) {
			game_mode = CREDITS;
			scene.StopAllMusic();
			scene.SetExposure(0.5f);
			credits_music.play();
		} else if (scene.GetMode() == Scene::MIDPOINT && game_mode != MIDPOINT) {
			game_mode = MIDPOINT;
			scene.StopAllMusic();
			scene.SetExposure(0.5f);
			credits_music.play();
		}

	
		//Main game update
		if (game_mode == MAIN_MENU || game_mode == ABOUT || game_mode == LEVELS || game_mode == SCREEN_SAVER || game_mode == CONTROLS) 
		{
			scene.UpdateCamera();
		}
		else if (game_mode == PLAYING || game_mode == CREDITS || game_mode == MIDPOINT || game_mode == LEVEL_EDITOR)
		{
			//Collect keyboard input
			const float force_lr =
				(all_keys[sf::Keyboard::Left] || all_keys[sf::Keyboard::A] ? -1.0f : 0.0f) +
				(all_keys[sf::Keyboard::Right] || all_keys[sf::Keyboard::D] ? 1.0f : 0.0f);
			const float force_ud =
				(all_keys[sf::Keyboard::Down] || all_keys[sf::Keyboard::S] ? -1.0f : 0.0f) +
				(all_keys[sf::Keyboard::Up] || all_keys[sf::Keyboard::W] ? 1.0f : 0.0f);

			//Apply forces to marble and camera
			scene.UpdateMarble(force_lr, force_ud);


			scene.free_camera_speed *= 1 + mouse_wheel * 0.05;

			//make ATB impossible to use while playing
			if (game_mode == PLAYING)
			{
				overlays.TWBAR_ENABLED = false;
			}

			sf::Vector2i mouse_delta = sf::Vector2i(0, 0);
			float ms = SETTINGS.stg.mouse_sensitivity;
			//Collect mouse input
			if (overlays.TWBAR_ENABLED)
			{
				window.setMouseCursorVisible(true);
				if (mouse_clicked)
				{
					mouse_delta = mouse_pos - mouse_prev_pos;
				}
			}
			else
			{
				window.setMouseCursorVisible(false);
				mouse_delta = mouse_pos - sf::Vector2i(window.getSize().x*0.5, window.getSize().y*0.5);
				sf::Mouse::setPosition(sf::Vector2i(window.getSize().x*0.5, window.getSize().y*0.5), window);
			}

			const float cam_lr = float(-mouse_delta.x) * ms;
			const float cam_ud = float(-mouse_delta.y) * ms;
			const float cam_z = mouse_wheel * SETTINGS.stg.wheel_sensitivity;

			scene.UpdateCamera(cam_lr, cam_ud, cam_z, mouse_clicked);
		}

		bool skip_frame = false;
		if (lag_ms >= 1000.0f / target_fps) {
			//If there is too much lag, just do another frame of physics and skip the draw
			lag_ms -= 1000.0f / target_fps;
			skip_frame = true;
		}
		else 
		{
			window.setVerticalSyncEnabled(VSYNC);

			//Update the shader values
			if (game_mode != FIRST_START)
			{
				scene.WriteRenderer(rend);
				rend.camera.SetAspectRatio((float)window.getSize().x / (float)window.getSize().y);
				rend.SetOutputTexture(main_txt);
			}


			//Draw the fractal	
			if (game_mode != FIRST_START)
			{
				//Draw to the render texture
				rend.Render();

				//Draw render texture to main window
				sf::Sprite sprite(main_txt);
				sprite.setScale(float(window.getSize().x) / float(rend.variables["width"]),
					float(window.getSize().y) / float(rend.variables["height"]));
				window.draw(sprite);
			}
		
		}

		//Draw text overlays to the window
		 if (game_mode == PLAYING) {
			if (scene.GetMode() == Scene::ORBIT && scene.GetMarble().x() < 998.0f) {
				overlays.DrawLevelDesc(window, scene.level_copy.txt);
			}
			else if (scene.GetMode() == Scene::MARBLE && !scene.IsFreeCamera()) {
				overlays.DrawArrow(window, scene.GetGoalDirection());
			}
			if (!scene.HasCheats() || scene.GetCountdownTime() < 4 * 60) {
				overlays.DrawTimer(window, scene.GetCountdownTime(), scene.IsHighScore());
			}
			if (!scene.HasCheats() && scene.IsFullRun() && !scene.IsFreeCamera()) {
				overlays.DrawSumTime(window, scene.GetSumTime());
			}
			if (scene.HasCheats() && !scene.IsFreeCamera()) {
				overlays.DrawCheatsEnabled(window);
			}
			if (show_cheats) {
				overlays.DrawCheats(window);
			}
		}
		else if (game_mode == CREDITS) {
			overlays.DrawCredits(window, scene.IsFullRun(), scene.GetSumTime());
		}
		else if (game_mode == MIDPOINT) {
			overlays.DrawMidPoint(window, scene.IsFullRun(), scene.GetSumTime());
		}

		 
		//new interface render stuff
		io_state.dt = prev_s;
		io_state.time += io_state.dt;
		UpdateAllObjects(&window, io_state);
		window.setView(default_window_view);
		
		if (!skip_frame) {
			if (overlays.TWBAR_ENABLED)
				scene.Synchronize();
			overlays.DrawAntTweakBar();

			window.display();

			//If V-Sync is running higher than desired fps, slow down!
			const float s = clock.restart().asSeconds();
			prev_s = s;
			if (s > 0.0f) {
				smooth_fps = smooth_fps * 0.9f + std::min(1.0f / s, target_fps)*0.1f;
			}
			const float time_diff_ms = 1000.0f * (1.0f / target_fps - s);
			if (time_diff_ms > 0) {
				sf::sleep(sf::seconds(time_diff_ms / 1000.0f));
				lag_ms = std::max(lag_ms - time_diff_ms, 0.0f);
			}
			else if (time_diff_ms < 0) {
				lag_ms += std::max(-time_diff_ms, 0.0f);
			}
		}
	}

	RemoveAllObjects();
	scene.StopMusic();
	scene.levels.SaveScoresToFile();
	TwTerminate();

	/*
	#ifdef _DEBUG
	system("pause");
	#endif
	*/
	return 0;
}
