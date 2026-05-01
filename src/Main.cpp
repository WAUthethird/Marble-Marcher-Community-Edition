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
bool TOUCH_MODE = false;
bool DEBUG_BAR = false;

std::string configPath;

#if defined(_WIN32)
int WinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR lpCmdLine, int nCmdShow) {
#else
int main(int argc, char *argv[]) {
#endif
	//Load the music
	sf::Music menu_music;
	(void)menu_music.openFromFile(menu_ogg);
	menu_music.setLooping(true);
	sf::Music level_music[num_level_music];
	(void)level_music[0].openFromFile(level1_ogg);
	level_music[0].setLooping(true);
	(void)level_music[1].openFromFile(level2_ogg);
	level_music[1].setLooping(true);
	(void)level_music[2].openFromFile(level3_ogg);
	level_music[2].setLooping(true);
	(void)level_music[3].openFromFile(level4_ogg);
	level_music[3].setLooping(true);
	sf::Music credits_music;
	(void)credits_music.openFromFile(credits_ogg);
	credits_music.setLooping(true);

	configPath = SETTINGS.GetConfigPath();
	bool first_start = SETTINGS.Load(configPath + "/settings.bin");

	//all of the fonts
	Fonts fonts;
	LOCAL.LoadLocalsFromFolder(local_folder, &fonts);
	
	//all declarations
	sf::RenderWindow window;
  
	Renderer rend(main_config);
	GLuint framebuffer, main_txt, screenshot_txt;
	
	//Create the fractal scene
	Scene scene(level_music);
	//Create the old menus
	Overlays overlays(&scene);
	sf::Clock clock;
	float smooth_fps = target_fps;
	float lag_ms = 0.0f;
	mouse_pos = sf::Vector2i(0, 0);
	mouse_prev_pos = sf::Vector2i(0, 0);
  
	SetPointers(&window, &scene, &overlays, &rend, &main_txt, &screenshot_txt, &framebuffer);

	ApplySettings(nullptr);

	scene.levels.LoadLevelsFromFolder(level_folder);
	scene.levels.LoadMusicFromFolder(music_folder);
	scene.levels.LoadScoresFromFile(configPath + "/scores.bin");

	InitializeATBWindows(&smooth_fps, &target_fps);

	window.requestFocus();
	UpdateAspectRatio(window.getSize().x, window.getSize().y);
	//set window icon
	sf::Image icon;
	(void)icon.loadFromFile(icon_png); 
	window.setIcon(icon.getSize(), icon.getPixelsPtr());

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

	#define n_touch 5
	sf::Vector2i touch_xy[n_touch];
	sf::Vector2i touch_pxy[n_touch];
	bool touched[n_touch] = { false };

	sf::CircleShape touch_circle[n_touch], joystick;

	for (int i = 0; i < n_touch; i++)
	{
		touch_circle[i].setRadius(0);
		touch_circle[i].setFillColor(sf::Color(128, 128, 128, 200));
	}
	joystick.setRadius(0);
	joystick.setFillColor(sf::Color(128, 128, 128, 128));
	int joystick_finger = -1;
	int view_finger = -1;

	//Main loop
	while (window.isOpen())
	{
		window.clear(sf::Color::White);
		float mouse_wheel = 0.0f;
		mouse_prev_pos = mouse_pos;
		io_state.mouse_prev = sf::Vector2f(mouse_prev_pos.x, mouse_prev_pos.y);
		io_state.wheel = mouse_wheel;
		io_state.mouse_press[2] = false;
		io_state.mouse_press[0] = false;
		TOUCH_MODE = SETTINGS.stg.touch_mode;

		for (int i = 0; i < n_touch; i++)
		{
			//previous touch state
			touch_pxy[i] = touch_xy[i];
		}

		for (int i = 0; i < sf::Keyboard::KeyCount; i++)
		{
			io_state.key_press[i] = false;
		}

		for (int i = 0; i < sf::Joystick::AxisCount; i++)
		{
			io_state.axis_moved[i] = false;
		}

		for (int i = 0; i < sf::Joystick::ButtonCount; i++)
		{
			io_state.button_pressed[i] = false;
		}



		while (const std::optional event = window.pollEvent()) 
		{
			bool handled = overlays.TwManageEvent((sf::Event*)&event);	

			if (event->is<sf::Event::Closed>()) 
			{
				window.close();
				break;
			}

			else if (event->is<sf::Event::FocusLost>()) 
			{
				if (game_mode == PLAYING) 
				{
					PauseGame(window, &overlays, &scene);
				}
			}
			else if (const auto* resized = event->getIf<sf::Event::Resized>()) 
			{
				overlays.SetScale( std::max(float(resized->size.x), float(resized->size.y))/ 1280.0f);
				sf::FloatRect visibleArea(sf::Vector2f(0, 0), (sf::Vector2f)resized->size);
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
				if (TOUCH_MODE)
				{
					if (const auto* touchBegan = event->getIf<sf::Event::TouchBegan>())
					{
						touched[touchBegan->finger] = true;
						touch_pxy[touchBegan->finger] = sf::Vector2i(touchBegan->position.x, touchBegan->position.y);
						if (touchBegan->finger < n_touch)
						{
							touch_circle[touchBegan->finger].setRadius(60);
						}
						//the joystick half of the screen
						if(touchBegan->position.x < window.getSize().x/2)
						{ 
							if (joystick_finger < 0)
							{
								joystick_finger = touchBegan->finger;
								joystick.setRadius(120);
								joystick.setPosition(sf::Vector2f(touchBegan->position.x - joystick.getRadius(), touchBegan->position.y - joystick.getRadius()));
							}
						}
						else //the view half of the screen
						{
							if (view_finger < 0)
							{
								view_finger = touchBegan->finger;
							}
						}
					}
					if (const auto* touchEnded = event->getIf<sf::Event::TouchEnded>())
					{
						touched[touchEnded->finger] = false;
						if (touchEnded->finger < n_touch)
						{
							touch_circle[touchEnded->finger].setRadius(0);
						}
						if (joystick_finger == touchEnded->finger)
						{
							joystick_finger = -1;
							joystick.setRadius(0);
						}
						if (view_finger == touchEnded->finger)
						{
							view_finger = -1;
						}
					}
				}

				if (const auto* joystickButtonPressed = event->getIf<sf::Event::JoystickButtonPressed>())
				{
					io_state.buttons[joystickButtonPressed->button] = true;
					io_state.button_pressed[joystickButtonPressed->button] = true;

					if (joystickButtonPressed->button == (int)SETTINGS.stg.control_mapping[JOYSTICK_RESTART])
					{
						if (game_mode == PLAYING) {
							scene.ResetLevel();
						}
					}
					else if (joystickButtonPressed->button == (int)SETTINGS.stg.control_mapping[JOYSTICK_EXIT])
					{
						if (game_mode == PLAYING) {
							PauseGame(window, &overlays, &scene);
						}
					} 
					else if (joystickButtonPressed->button == (int)SETTINGS.stg.control_mapping[JOYSTICK_SCREENSHOT])
					{
						TakeScreenshot();
					}
				}
				else if (const auto* joystickButtonReleased = event->getIf<sf::Event::JoystickButtonReleased>())
				{
					io_state.buttons[joystickButtonReleased->button] = false;
				}
				else if (const auto* joystickMoved = event->getIf<sf::Event::JoystickMoved>())
				{
					io_state.axis_value[(int)joystickMoved->axis] = 
						(abs(joystickMoved->position)<SETTINGS.stg.gamepad_deadzone)?0.f:joystickMoved->position;
					io_state.axis_moved[(int)joystickMoved->axis] = true;
				}
				else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
				{
					const sf::Keyboard::Key keycode = keyPressed->code;
					all_keys[(int)keycode] = true;
					io_state.isKeyPressed = true;
					io_state.keys[(int)keycode] = true;
					io_state.key_press[(int)keycode] = true;  
					if ((int)keyPressed->code < 0 || (int)keyPressed->code >= sf::Keyboard::KeyCount) { continue; }
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
					else if (keycode == sf::Keyboard::Key::D && keyPressed->control)
					{
						DEBUG_BAR = !DEBUG_BAR;
						TwDefine((std::string("Debug_bar visible=") + ((DEBUG_BAR) ? "true" : "false")).c_str());
					}
					else if (keycode == sf::Keyboard::Key::Escape)
					{
						if (game_mode == MAIN_MENU)
						{
							if(NumberOfObjects() < 2)
								ConfirmExit(&scene, &overlays);
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
								ConfirmEditorExit(&scene, &overlays);
						}
					}
					else if (keycode == SETTINGS.stg.control_mapping[RESTART])
					{
						if (game_mode == PLAYING) {
							scene.ResetLevel();
						}
					}
					else if (keycode == SETTINGS.stg.control_mapping[PAUSE])
					{
						if (game_mode == PLAYING) {
							PauseGame(window, &overlays, &scene);
						}
					}
					else if (keycode == SETTINGS.stg.control_mapping[ZOOM_IN])
					{
						if (game_mode == PLAYING) {
							mouse_wheel += 1.f;
						}
					}
					else if (keycode == SETTINGS.stg.control_mapping[ZOOM_OUT])
					{
						if (game_mode == PLAYING) {
							mouse_wheel -= 1.f;
						}
					}
					else if (keycode == sf::Keyboard::Key::F1) 
					{
						if (game_mode == PLAYING) {
							show_cheats = !show_cheats;
							scene.EnbaleCheats();
						}
					}
					else if (keycode == SETTINGS.stg.control_mapping[SCREENSHOT]) 
					{ 
						TakeScreenshot();
					} 
					else if (keycode == sf::Keyboard::Key::F4)
					{
						overlays.TWBAR_ENABLED = !overlays.TWBAR_ENABLED;
					} 
					else if (keycode == sf::Keyboard::Key::C) 
					{
						scene.Cheat_ColorChange();
					} 	
					else if (keycode == sf::Keyboard::Key::F) 
					{
						scene.Cheat_FreeCamera();
					}
					else if (keycode == sf::Keyboard::Key::G) 
					{
						scene.Cheat_Gravity();
					}
					else if (keycode == sf::Keyboard::Key::H) 
					{
						scene.Cheat_HyperSpeed();
					}
					else if (keycode == sf::Keyboard::Key::I) 
					{
						scene.Cheat_IgnoreGoal();
					}
					else if (keycode == sf::Keyboard::Key::M) 
					{
						scene.Cheat_Motion();
					}
					else if (keycode == sf::Keyboard::Key::P) 
					{
						scene.Cheat_Planet();
					}
					else if (keycode == sf::Keyboard::Key::Z) 
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
					if (keycode >= sf::Keyboard::Key::Num0 && keycode <= sf::Keyboard::Key::Num9)
					{
						scene.Cheat_Param(int(keycode) - int(sf::Keyboard::Key::Num1));
					}
				
				}
				else if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>()) 
				{
					const sf::Keyboard::Key keycode = keyReleased->code;
					if ((int)keyReleased->code < 0 || (int)keyReleased->code >= sf::Keyboard::KeyCount) { continue; }
					all_keys[(int)keycode] = false;
					io_state.keys[(int)keycode] = false;
				}
				else if (const auto* mouseButtonPressed = event->getIf<sf::Event::MouseButtonPressed>())
			    {
					if (mouseButtonPressed->button == sf::Mouse::Button::Left) 
					{
						mouse_pos = sf::Vector2i(mouseButtonPressed->position.x, mouseButtonPressed->position.y);
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
					else if (mouseButtonPressed->button == sf::Mouse::Button::Right && !TOUCH_MODE)
					{
						io_state.mouse[2] = true; 
						io_state.mouse_press[2] = true;
						if (game_mode == PLAYING) {
							scene.ResetLevel();
						}
					}
				}
				else if (const auto* mouseButtonReleased = event->getIf<sf::Event::MouseButtonReleased>())
			    {
					if (mouseButtonReleased->button == sf::Mouse::Button::Left) 
					{
						io_state.mouse[0] = false;
						mouse_pos = sf::Vector2i(mouseButtonReleased->position.x, mouseButtonReleased->position.y);
						mouse_clicked = false;
					}
					else if (mouseButtonReleased->button == sf::Mouse::Button::Right) {
						io_state.mouse[2] = false;
					}
				}
				else if (const auto* mouseMoved = event->getIf<sf::Event::MouseMoved>()) 
				{
					mouse_pos = sf::Vector2i(mouseMoved->position.x, mouseMoved->position.y);
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
				else if (const auto* mouseWheelScrolled = event->getIf<sf::Event::MouseWheelScrolled>())
			    {
					mouse_wheel += mouseWheelScrolled->delta;
					io_state.wheel = mouse_wheel;
				}
			}
		}

		if (TOUCH_MODE)
		{
			int touches = 0;
			for (int i = 0; i < n_touch; i++)
			{
				if (touched[i])
				{
					touches++;
					//touch state
					touch_xy[i] = sf::Touch::getPosition(i, window);
					touch_circle[i].setPosition(sf::Vector2f(touch_xy[i].x - touch_circle[i].getRadius(), touch_xy[i].y - touch_circle[i].getRadius()));
				}
			}

			if (touches == 3)
			{
				if (game_mode == PLAYING)
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
					else if (get_glob_obj(focused).action_time < 0.f)//remove confirm window
					{
						RemoveGlobalObject(focused);
					}
				}
			}
		}
	
		//Check if the game was beat
		if (scene.GetMode() == Scene::FINAL && game_mode != CREDITS) {
			game_mode = CREDITS;
			scene.StopAllMusic();
			scene.SetExposure(0.5f);
			credits_music.play();
			credits_music.setVolume(SETTINGS.stg.music_volume);
		} else if (scene.GetMode() == Scene::MIDPOINT && game_mode != MIDPOINT) {
			game_mode = MIDPOINT;
			scene.StopAllMusic();
			scene.SetExposure(0.5f);
			credits_music.play();
			credits_music.setVolume(SETTINGS.stg.music_volume);
		}
		
	
		//Main game update
		if (game_mode == MAIN_MENU || game_mode == ABOUT || game_mode == LEVELS || game_mode == SCREEN_SAVER || game_mode == CONTROLS) 
		{
			scene.UpdateCamera();
		}
		else if (game_mode == PLAYING || game_mode == CREDITS || game_mode == MIDPOINT || game_mode == LEVEL_EDITOR)
		{
			float force_x = 0, force_y = 0;
			if (TOUCH_MODE)
			{
				sf::Vector2i djoy = sf::Vector2i(0,0);
				//Collect touch joystick input
				if (joystick_finger >= 0)
				{
					djoy = touch_xy[joystick_finger] - (sf::Vector2i(joystick.getPosition()) + sf::Vector2i(joystick.getRadius(), joystick.getRadius()));
					force_x = float(djoy.x) / float(joystick.getRadius());
					force_y = -float(djoy.y) / float(joystick.getRadius());
				}
			}
			else
			{
				//Collect keyboard input
				force_y = (all_keys[(int)SETTINGS.stg.control_mapping[DOWN]] ? -1.0f : 0.0f) +
					      (all_keys[(int)SETTINGS.stg.control_mapping[UP]] ? 1.0f : 0.0f);
				force_x = (all_keys[(int)SETTINGS.stg.control_mapping[LEFT]] ? -1.0f : 0.0f) +
						  (all_keys[(int)SETTINGS.stg.control_mapping[RIGHT]] ? 1.0f : 0.0f);
			}
			//Collect gamepad input
			force_y -= io_state.axis_value[(int)SETTINGS.stg.control_mapping[JOYSTICK_MOVE_AXIS_Y]];
			force_x += io_state.axis_value[(int)SETTINGS.stg.control_mapping[JOYSTICK_MOVE_AXIS_X]];
			 
			InputRecord record = GetRecord();

			if (replay)
			{
				force_x = record.move_x;
				force_y = record.move_y;
			}
			scene.UpdateMarble(force_x, force_y);
			scene.free_camera_speed *= 1 + mouse_wheel * 0.05;

			//make ATB impossible to use while playing
			if (game_mode == PLAYING)
			{
				overlays.TWBAR_ENABLED = false;
			}

			sf::Vector2i mouse_delta = sf::Vector2i(0, 0);
			float ms = SETTINGS.stg.mouse_sensitivity;
			//Collect mouse/touch input
			if (view_finger >= 0 && TOUCH_MODE)
			{
				mouse_delta = touch_xy[view_finger] - touch_pxy[view_finger];
			}
			else if (overlays.TWBAR_ENABLED && !TOUCH_MODE)
			{
				window.setMouseCursorVisible(true);
				if (mouse_clicked)
				{
					mouse_delta = mouse_pos - mouse_prev_pos;
				}
			}
			else if(!TOUCH_MODE)
			{
				window.setMouseCursorVisible(false);
				mouse_delta = mouse_pos - sf::Vector2i(window.getSize().x*0.5, window.getSize().y*0.5);
				sf::Mouse::setPosition(sf::Vector2i(window.getSize().x*0.5, window.getSize().y*0.5), window);
			}

			float cam_lr = float(-mouse_delta.x) * ms;
			float cam_ud = float(-mouse_delta.y) * ms;
			cam_ud -= 0.05* ms *io_state.axis_value[(int)SETTINGS.stg.control_mapping[JOYSTICK_VIEW_AXIS_Y]];
			cam_lr -= 0.05* ms *io_state.axis_value[(int)SETTINGS.stg.control_mapping[JOYSTICK_VIEW_AXIS_X]];

			cam_ud +=  5*ms * ( (all_keys[(int)SETTINGS.stg.control_mapping[VIEWUP]] ? -1.0f : 0.0f) +
						    	(all_keys[(int)SETTINGS.stg.control_mapping[VIEWDOWN]] ? 1.0f : 0.0f) );
			cam_lr +=  5*ms * ((all_keys[(int)SETTINGS.stg.control_mapping[VIEWRIGHT]] ? -1.0f : 0.0f) +
								(all_keys[(int)SETTINGS.stg.control_mapping[VIEWLEFT]] ? 1.0f : 0.0f));
			
			float cam_z = mouse_wheel * SETTINGS.stg.wheel_sensitivity;

			if (replay)
			{
				cam_lr = record.view_x;
				cam_ud = record.view_y;
				cam_z = record.cam_z;
				mouse_clicked = record.mouse_clicked;
			}

			scene.UpdateCamera(cam_lr, cam_ud, cam_z, mouse_clicked);

			SaveRecord(force_x, force_y, cam_lr, cam_ud, cam_z, mouse_clicked);
		}

		bool skip_frame = false;
		if ((lag_ms >= 1000.0f / target_fps) && SETTINGS.stg.speed_regulation) {
			//If there is too much lag, just do another frame of physics and skip the draw
			lag_ms -= 1000.0f / target_fps;
			skip_frame = true;
		}
		else 
		{
			//Update the shader values
			if (game_mode != FIRST_START)
			{
				if (!(taken_screenshot && SETTINGS.stg.screenshot_preview))
				{
					glNamedFramebufferTexture(framebuffer, GL_COLOR_ATTACHMENT0, main_txt, 0);
					scene.WriteRenderer(rend);
					rend.camera.SetAspectRatio((float)window.getSize().x / (float)window.getSize().y);
					rend.SetOutputTexture(main_txt);
					//Draw to the render texture
					rend.Render();
					window.resetGLStates();
					//Draw render texture to main window
					glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
					glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
					glBlitFramebuffer(rend.variables["width"], 0, 0, rend.variables["height"], window.getSize().x, window.getSize().y, 0, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
				}
				else
				{
/*					//Draw screenshot preview
					sf::Sprite sprite(screenshot_txt);
					sf::Vector2u ssize = screenshot_txt.getSize();
					float scale = min(float(window.getSize().x) / float(ssize.x),
						float(window.getSize().y) / float(ssize.y));
					vec2 pos = vec2(window.getSize().x - ssize.x*scale, window.getSize().y - ssize.y*scale)*0.5f;
					sprite.setScale(sf::Vector2f(scale, scale));
					sprite.setPosition(sf::Vector2f(pos.x, pos.y));
					window.draw(sprite);
*/
					const float s = screenshot_clock.getElapsedTime().asSeconds();
					if (s > SETTINGS.stg.preview_time)
					{
						taken_screenshot = false;
					}
				}
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
		io_state.isKeyPressed = false;
		window.setView(default_window_view);
		UpdateUniforms();
		
		if (!skip_frame) {
			if (overlays.TWBAR_ENABLED)
				scene.Synchronize();
			overlays.DrawAntTweakBar();

			if (TOUCH_MODE)
			{
				for (int i = 0; i < n_touch; i++)
				{
					window.draw(touch_circle[i]);
				}
				if (game_mode == PLAYING || game_mode == CREDITS || game_mode == MIDPOINT || game_mode == LEVEL_EDITOR)
				{
					window.draw(joystick);
		    	}
			}

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
	scene.levels.SaveScoresToFile(configPath + "/scores.bin");
	TwTerminate();
	return 0;
}