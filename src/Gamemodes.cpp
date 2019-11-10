#include "Gamemodes.h"


//Global variables
sf::Vector2i mouse_pos, mouse_prev_pos;
InputState io_state;

bool fullscreen_current = false;
bool all_keys[sf::Keyboard::KeyCount] = { 0 };
bool mouse_clicked = false;
bool show_cheats = false;
bool taken_screenshot = false;
sf::Clock screenshot_clock;
//Constants

float target_fps = 60.0f;

GameMode game_mode = MAIN_MENU;

Scene *scene_ptr;
Overlays *overlays_ptr;
Renderer *renderer_ptr;
sf::RenderWindow *window;
sf::Texture *main_txt;
sf::Texture *screenshot_txt;

void SetPointers(sf::RenderWindow *w, Scene* scene, Overlays* overlays, Renderer* rd, sf::Texture *main, sf::Texture *screensht)
{
	window = w;
	scene_ptr = scene;
	overlays_ptr = overlays;
	renderer_ptr = rd;
	main_txt = main;
	screenshot_txt = screensht;
}

void OpenMainMenu(Scene * scene, Overlays * overlays)
{
	RemoveAllObjects();
	game_mode = MAIN_MENU;

	scene->SetCurrentMusic(scene->levels.GetMusic("menu.ogg"));

	scene->SetExposure(1.0f);
	scene->SetMode(Scene::INTRO);
	sf::Vector2f wsize = default_size;
	sf::Vector2f vsize = default_view.getSize();
	MenuBox mainmenu(1000, vsize.y*0.95f, false, wsize.x*0.025, wsize.y*0.025f);
	mainmenu.SetBackgroundColor(sf::Color::Transparent);
	//make the menu static
	mainmenu.static_object = true;

	//TITLE
	Text ttl("Marble\nMarcher", LOCAL("default"), 120, sf::Color::White);
	ttl.SetBorderColor(sf::Color::Black);
	ttl.SetBorderWidth(4);
	mainmenu.AddObject(&ttl, Object::Allign::LEFT);

	Box margin1(800, 5);
	margin1.SetBackgroundColor(sf::Color::Transparent);
	mainmenu.AddObject(&margin1, Object::Allign::LEFT);

	Text CE("Community Edition", LOCAL("default"), 60, sf::Color::White);
	CE.SetBorderColor(sf::Color::Black);
	CE.SetBorderWidth(4);
	mainmenu.AddObject(&CE, Object::Allign::LEFT);

	Box margin(800, 80);
	margin.SetBackgroundColor(sf::Color::Transparent);
	mainmenu.AddObject(&margin, Object::Allign::LEFT);

	//PLAY
	Box playbtn(600, 50);
	Text button1(LOCAL["Play"], LOCAL("default"), 40, sf::Color::White);
	button1.SetBorderColor(sf::Color::Black);
	playbtn.hoverstate.color_main = sf::Color(200, 40, 0, 255);
	playbtn.SetCallbackFunction([scene, overlays](sf::RenderWindow * window, InputState & state)
	{
		PlayNewGame(scene, window, 0);
		overlays->sound_click.play();
	}, true);
	playbtn.AddObject(&button1, Object::Allign::CENTER);
	mainmenu.AddObject(&playbtn, Object::Allign::LEFT);

	//LEVELS
	Box lvlsbtn(600, 50);
	Text button2(LOCAL["Levels"], LOCAL("default"), 40, sf::Color::White);
	button2.SetBorderColor(sf::Color::Black);
	lvlsbtn.hoverstate.color_main = sf::Color(200, 40, 0, 255);
	lvlsbtn.SetCallbackFunction([scene, overlays](sf::RenderWindow * window, InputState & state)
	{
		OpenLevelMenu(scene, overlays);
		overlays->sound_click.play();
	}, true);
	lvlsbtn.AddObject(&button2, Object::Allign::CENTER);
	mainmenu.AddObject(&lvlsbtn, Object::Allign::LEFT);

	//Settings
	Box sttbtn(600, 50);
	Text buttonstt(LOCAL["Settings"], LOCAL("default"), 40, sf::Color::White);
	buttonstt.SetBorderColor(sf::Color::Black);
	sttbtn.hoverstate.color_main = sf::Color(200, 40, 0, 255);
	sttbtn.SetCallbackFunction([scene, overlays](sf::RenderWindow * window, InputState & state)
	{
		overlays->TWBAR_ENABLED = !overlays->TWBAR_ENABLED;
		TwDefine("Settings iconified=false");
		overlays->sound_click.play();
	}, true);
	sttbtn.AddObject(&buttonstt, Object::Allign::CENTER);
	mainmenu.AddObject(&sttbtn, Object::Allign::LEFT);
	
	//Controls
	Box cntrlbtn(600, 50);
	Text button3(LOCAL["Controls"], LOCAL("default"), 40, sf::Color::White);
	button3.SetBorderColor(sf::Color::Black);
	cntrlbtn.hoverstate.color_main = sf::Color(200, 40, 0, 255);
	cntrlbtn.SetCallbackFunction([scene, overlays](sf::RenderWindow * window, InputState & state)
	{
		OpenControlMenu(scene, overlays);
		overlays->sound_click.play();
	}, true);
	cntrlbtn.AddObject(&button3, Object::Allign::CENTER);
	mainmenu.AddObject(&cntrlbtn, Object::Allign::LEFT);

	//Credits
	Box aboutbtn(600, 50);
	Text button5(LOCAL["Credits"], LOCAL("default"), 40, sf::Color::White);
	button5.SetBorderColor(sf::Color::Black);
	aboutbtn.hoverstate.color_main = sf::Color(200, 40, 0, 255);
	aboutbtn.SetCallbackFunction([scene, overlays](sf::RenderWindow * window, InputState & state)
	{
		OpenCredits(scene, overlays);
		overlays->sound_click.play();
	}, true);
	aboutbtn.AddObject(&button5, Object::Allign::CENTER);
	mainmenu.AddObject(&aboutbtn, Object::Allign::LEFT);


	//Screen Saver
	Box ssbtn(600, 50);
	Text button4(LOCAL["Screen_Saver"], LOCAL("default"), 40, sf::Color::White);
	button4.SetBorderColor(sf::Color::Black);
	ssbtn.hoverstate.color_main = sf::Color(200, 40, 0, 255);
	ssbtn.SetCallbackFunction([scene, overlays](sf::RenderWindow * window, InputState & state)
	{
		OpenScreenSaver(scene, overlays);
		overlays->sound_click.play();
	}, true);
	ssbtn.AddObject(&button4, Object::Allign::CENTER);
	mainmenu.AddObject(&ssbtn, Object::Allign::LEFT);

	//Exit
	Box exitbtn(600, 50);
	Text button6(LOCAL["Exit"], LOCAL("default"), 40, sf::Color::White);
	button6.SetBorderColor(sf::Color::Black);
	exitbtn.hoverstate.color_main = sf::Color(200, 40, 0, 255);
	exitbtn.SetCallbackFunction([scene, overlays](sf::RenderWindow * window, InputState & state)
	{
		overlays->sound_click.play();
		window->close();
	}, true);
	exitbtn.AddObject(&button6, Object::Allign::CENTER);
	mainmenu.AddObject(&exitbtn, Object::Allign::LEFT);

	Text about(LOCAL["About"], LOCAL("mono"), 30, sf::Color::White);
	about.SetBorderColor(sf::Color::Black);
	about.SetBorderWidth(3);
	mainmenu.AddObject(&about, Object::Allign::LEFT);
	

	AddGlobalObject(mainmenu);
}


void OpenCredits(Scene * scene, Overlays * overlays)
{
	RemoveAllObjects();
	game_mode = ABOUT;

	scene->SetCurrentMusic(scene->levels.GetMusic("menu.ogg"));
	scene->SetExposure(1.0f);
	scene->SetMode(Scene::INTRO);

	sf::Vector2f wsize = default_size;
	sf::Vector2f vsize = default_view.getSize();
	MenuBox creditslist(wsize.x*0.95f, vsize.y*0.95f, false, (vsize.x - wsize.x*0.95f) / 2, vsize.y*0.025f);

	//add a default callback
	creditslist.SetDefaultFunction([scene, overlays](sf::RenderWindow * window, InputState & state)
	{
		if (state.keys[sf::Keyboard::Escape])
		{
			OpenMainMenu(scene, overlays);
		}
	});

	creditslist.SetBackgroundColor(sf::Color(30,30,30,200));
	//make it static
	creditslist.static_object = true;

	//TITLE
	Text ttl(LOCAL["Credits"], LOCAL("default"), 60, sf::Color::White);
	ttl.SetBorderColor(sf::Color::Black);
	ttl.SetBorderWidth(4);
	creditslist.AddObject(&ttl, Object::Allign::CENTER);

	Button back(LOCAL["Back2Main"], 600, 50, 
		[scene, overlays](sf::RenderWindow * window, InputState & state)
		{
		 	OpenMainMenu(scene, overlays);
			overlays->sound_click.play();
		},
		sf::Color(200, 40, 0, 255), sf::Color(128, 128, 128, 128));
	creditslist.AddObject(&back, Object::Allign::LEFT);

	//Credits list
	std::vector<std::vector<std::string>> credits_list = {
		{"images/credits/codeparade.jpg" , "Codeparade" , "Codeparade_descr"},
		{"images/credits/michaelmoroz.jpg" , "Michael Moroz" , "Michael_descr"},
		{"images/credits/wauthethird.png" , "WAUthethird" , "WAU_descr"},
		{"images/credits/Bryce.png" , "Bryce AS202313" , "Bryce_descr"} 
	};

	for (auto &str : credits_list)
	{
		Box credits_entry_1(wsize.x*0.95f - 60, 120);
		credits_entry_1.SetBackgroundColor(sf::Color(128, 128, 128, 128));
		credits_entry_1.AddObject(new Image(str[0], 116, 116), Object::Allign::LEFT);
		Box credits_text_1(wsize.x*0.95f - 200, 116, sf::Color::Transparent);
		credits_text_1.AddObject(new Text(LOCAL[str[1]], LOCAL("default"), 50), Object::Allign::LEFT);
		credits_text_1.AddObject(new Box(0, 50), Object::Allign::CENTER);
		credits_text_1.AddObject(new Text(LOCAL[str[2]], LOCAL("default"), 30), Object::Allign::LEFT);
		credits_entry_1.AddObject(&credits_text_1, Object::Allign::LEFT);
		creditslist.AddObject(&credits_entry_1, Object::Allign::LEFT);
	}

	AddGlobalObject(creditslist);
}

void OpenEditor(Scene * scene, Overlays * overlays, int level)
{
	StopReplay();
	scene->StopMusic();
	RemoveAllObjects();
	//go to level editor
	game_mode = LEVEL_EDITOR;
	scene->SetExposure(1.0f);
	overlays->TWBAR_ENABLED = true;
	TwDefine("LevelEditor visible=true position='20 20'");
	TwDefine("FractalEditor visible=true position='20 500'");
	TwDefine("Settings iconified=true");
	TwDefine("Statistics iconified=true");
	scene->StartLevelEditor(level);
}

void PlayLevel(Scene * scene, sf::RenderWindow * window, int level)
{
	StopReplay();
	RemoveAllObjects();
	//play level
	game_mode = PLAYING;
	scene->StartSingle(level);
	LockMouse(*window);
}

void RePlayBest(Scene * scene, sf::RenderWindow * window, int level)
{
	StopReplay();
	RemoveAllObjects();
	//play level
	game_mode = PLAYING;
	scene->ReplayLevel(level);
	LockMouse(*window);
}


void OpenControlMenu(Scene * scene, Overlays * overlays)
{
	RemoveAllObjects();
	game_mode = CONTROLS;

	scene->SetCurrentMusic(scene->levels.GetMusic("menu.ogg"));
	scene->SetExposure(1.0f);
	scene->SetMode(Scene::INTRO);

	sf::Vector2f wsize = default_size;
	sf::Vector2f vsize = default_view.getSize();
	MenuBox controls(wsize.x*0.95f, vsize.y*0.95f, false, (vsize.x - wsize.x*0.95f) / 2, vsize.y*0.025f);

	//add a default callback
	controls.SetDefaultFunction([scene, overlays](sf::RenderWindow * window, InputState & state)
	{
		if (state.keys[sf::Keyboard::Escape])
		{
			OpenMainMenu(scene, overlays);
		}
	});

	controls.SetBackgroundColor(sf::Color(30, 30, 30, 200));
	//make it static
	controls.static_object = true;

	//TITLE
	Text ttl(LOCAL["Controls"], LOCAL("default"), 60, sf::Color::White);
	ttl.SetBorderColor(sf::Color::Black);
	ttl.SetBorderWidth(4);
	controls.AddObject(&ttl, Object::Allign::CENTER);

	controls.AddObject(new Button(LOCAL["Back2Main"], 600, 50,
		[scene, overlays](sf::RenderWindow * window, InputState & state)
		{
			OpenMainMenu(scene, overlays);
			overlays->sound_click.play();
		},
		sf::Color(200, 40, 0, 255), sf::Color(128, 128, 128, 128)), Object::Allign::LEFT);

	controls.AddObject(new Box(800, 0), Object::Allign::CENTER);

	controls.AddObject(new Text(LOCAL["Keyboard Controls"], LOCAL("default"), 50), Object::Allign::LEFT);
	controls.AddObject(new Box(800, 0), Object::Allign::CENTER);
	controls.AddObject(new KeyMapper(LOCAL["UP"], LOCAL["Waiting for input"], &SETTINGS.stg.control_mapping[UP], 800.f, 35.f, KeyMapper::MapperType::KEYBOARD), Object::Allign::LEFT);
	controls.AddObject(new KeyMapper(LOCAL["DOWN"], LOCAL["Waiting for input"], &SETTINGS.stg.control_mapping[DOWN], 800.f, 35.f, KeyMapper::MapperType::KEYBOARD), Object::Allign::LEFT);
	controls.AddObject(new KeyMapper(LOCAL["LEFT"], LOCAL["Waiting for input"], &SETTINGS.stg.control_mapping[LEFT], 800.f, 35.f, KeyMapper::MapperType::KEYBOARD), Object::Allign::LEFT);
	controls.AddObject(new KeyMapper(LOCAL["RIGHT"], LOCAL["Waiting for input"], &SETTINGS.stg.control_mapping[RIGHT], 800.f, 35.f, KeyMapper::MapperType::KEYBOARD), Object::Allign::LEFT);
	controls.AddObject(new KeyMapper(LOCAL["VIEWUP"], LOCAL["Waiting for input"], &SETTINGS.stg.control_mapping[VIEWUP], 800.f, 35.f, KeyMapper::MapperType::KEYBOARD), Object::Allign::LEFT);
	controls.AddObject(new KeyMapper(LOCAL["VIEWDOWN"], LOCAL["Waiting for input"], &SETTINGS.stg.control_mapping[VIEWDOWN], 800.f, 35.f, KeyMapper::MapperType::KEYBOARD), Object::Allign::LEFT);
	controls.AddObject(new KeyMapper(LOCAL["VIEWLEFT"], LOCAL["Waiting for input"], &SETTINGS.stg.control_mapping[VIEWLEFT], 800.f, 35.f, KeyMapper::MapperType::KEYBOARD), Object::Allign::LEFT);
	controls.AddObject(new KeyMapper(LOCAL["VIEWRIGHT"], LOCAL["Waiting for input"], &SETTINGS.stg.control_mapping[VIEWRIGHT], 800.f, 35.f, KeyMapper::MapperType::KEYBOARD), Object::Allign::LEFT);
	controls.AddObject(new KeyMapper(LOCAL["PAUSE"], LOCAL["Waiting for input"], &SETTINGS.stg.control_mapping[PAUSE], 800.f, 35.f, KeyMapper::MapperType::KEYBOARD), Object::Allign::LEFT);
	controls.AddObject(new KeyMapper(LOCAL["RESTART"], LOCAL["Waiting for input"], &SETTINGS.stg.control_mapping[RESTART], 800.f, 35.f, KeyMapper::MapperType::KEYBOARD), Object::Allign::LEFT);
	controls.AddObject(new KeyMapper(LOCAL["SCREENSHOT"], LOCAL["Waiting for input"], &SETTINGS.stg.control_mapping[SCREENSHOT], 800.f, 35.f, KeyMapper::MapperType::KEYBOARD), Object::Allign::LEFT);
	controls.AddObject(new KeyMapper(LOCAL["ZOOM_IN"], LOCAL["Waiting for input"], &SETTINGS.stg.control_mapping[ZOOM_IN], 800.f, 35.f, KeyMapper::MapperType::KEYBOARD), Object::Allign::LEFT);
	controls.AddObject(new KeyMapper(LOCAL["ZOOM_OUT"], LOCAL["Waiting for input"], &SETTINGS.stg.control_mapping[ZOOM_OUT], 800.f, 35.f, KeyMapper::MapperType::KEYBOARD), Object::Allign::LEFT);
	controls.AddObject(new Box(800, 0), Object::Allign::CENTER);
	controls.AddObject(new Text(LOCAL["Gamepad Controls"], LOCAL("default"), 50), Object::Allign::LEFT);
	controls.AddObject(new Box(800, 0), Object::Allign::CENTER);
	controls.AddObject(new KeyMapper(LOCAL["JOYSTICK_MOVE_AXIS_X"], LOCAL["Waiting for input"], &SETTINGS.stg.control_mapping[JOYSTICK_MOVE_AXIS_X], 800.f, 35.f, KeyMapper::MapperType::JOYSTICK_AXIS), Object::Allign::LEFT);
	controls.AddObject(new KeyMapper(LOCAL["JOYSTICK_MOVE_AXIS_Y"], LOCAL["Waiting for input"], &SETTINGS.stg.control_mapping[JOYSTICK_MOVE_AXIS_Y], 800.f, 35.f, KeyMapper::MapperType::JOYSTICK_AXIS), Object::Allign::LEFT);
	controls.AddObject(new KeyMapper(LOCAL["JOYSTICK_VIEW_AXIS_X"], LOCAL["Waiting for input"], &SETTINGS.stg.control_mapping[JOYSTICK_VIEW_AXIS_X], 800.f, 35.f, KeyMapper::MapperType::JOYSTICK_AXIS), Object::Allign::LEFT);
	controls.AddObject(new KeyMapper(LOCAL["JOYSTICK_VIEW_AXIS_Y"], LOCAL["Waiting for input"], &SETTINGS.stg.control_mapping[JOYSTICK_VIEW_AXIS_Y], 800.f, 35.f, KeyMapper::MapperType::JOYSTICK_AXIS), Object::Allign::LEFT);
	controls.AddObject(new KeyMapper(LOCAL["JOYSTICK_EXIT"], LOCAL["Waiting for input"], &SETTINGS.stg.control_mapping[JOYSTICK_EXIT], 800.f, 35.f, KeyMapper::MapperType::JOYSTICK_KEYS), Object::Allign::LEFT);
	controls.AddObject(new KeyMapper(LOCAL["JOYSTICK_SCREENSHOT"], LOCAL["Waiting for input"], &SETTINGS.stg.control_mapping[JOYSTICK_SCREENSHOT], 800.f, 35.f, KeyMapper::MapperType::JOYSTICK_KEYS), Object::Allign::LEFT);
	controls.AddObject(new KeyMapper(LOCAL["JOYSTICK_RESTART"], LOCAL["Waiting for input"], &SETTINGS.stg.control_mapping[JOYSTICK_RESTART], 800.f, 35.f, KeyMapper::MapperType::JOYSTICK_KEYS), Object::Allign::LEFT);

	AddGlobalObject(controls);
}

void ResumeGame(sf::RenderWindow &window)
{
	RemoveAllObjects();
	game_mode = PLAYING;
	LockMouse(window);
}


void OpenPauseMenu(Scene * scene, Overlays * overlays)
{
	RemoveAllObjects();
	scene->SetExposure(1.0f);
	sf::Vector2f wsize = default_size;
	sf::Vector2f vsize = default_view.getSize();
	MenuBox pausemenu(625, 640, false, wsize.x*0.025, wsize.y*0.025f);
	pausemenu.SetBackgroundColor(sf::Color(32, 32, 32, 200));
	
	//make the menu static
	pausemenu.static_object = true;

	//TITLE
	Text ttl(LOCAL["Paused"], LOCAL("default"), 120, sf::Color::White);
	ttl.SetBorderColor(sf::Color::Black);
	ttl.SetBorderWidth(4);
	pausemenu.AddObject(&ttl, Object::Allign::LEFT);

	Box margin1(600, 0);
	margin1.SetBackgroundColor(sf::Color::Transparent);
	pausemenu.AddObject(&margin1, Object::Allign::LEFT);

	Text CE1(scene->level_copy.txt, LOCAL("default"), 40, sf::Color::White);
	CE1.SetBorderColor(sf::Color::Black);
	CE1.SetBorderWidth(0);
	pausemenu.AddObject(&CE1, Object::Allign::LEFT);

	pausemenu.AddObject(&margin1, Object::Allign::LEFT);

	Text CE2(scene->level_copy.desc, LOCAL("default"), 20, sf::Color::White);
	CE2.SetBorderColor(sf::Color::Black);
	CE2.SetBorderWidth(0);
	pausemenu.AddObject(&CE2, Object::Allign::LEFT);

	Box margin(600, 20);
	margin.SetBackgroundColor(sf::Color::Transparent);
	pausemenu.AddObject(&margin, Object::Allign::LEFT);

	
	Box resumebtn(600, 50);
	Text button1(LOCAL["Resume"], LOCAL("default"), 40, sf::Color::White);
	button1.SetBorderColor(sf::Color::Black);
	button1.SetBorderWidth(2);
	resumebtn.SetBackgroundColor(sf::Color(200, 200, 200, 200));
	resumebtn.hoverstate.color_main = sf::Color(200, 40, 0, 255);
	resumebtn.SetCallbackFunction([scene, overlays](sf::RenderWindow * window, InputState & state)
	{
		RemoveAllObjects();
		game_mode = PLAYING;
		scene->SetExposure(1.0f);
		LockMouse(*window);
		overlays->sound_click.play();
	}, true);
	resumebtn.AddObject(&button1, Object::Allign::CENTER);
	pausemenu.AddObject(&resumebtn, Object::Allign::LEFT);


	Box rstbtn(600, 50);
	Text button2(LOCAL["Restart"], LOCAL("default"), 40, sf::Color::White);
	button2.SetBorderColor(sf::Color::Black);
	button2.SetBorderWidth(2);
	rstbtn.SetBackgroundColor(sf::Color(200, 200, 200, 200));
	rstbtn.hoverstate.color_main = sf::Color(200, 40, 0, 255);
	rstbtn.SetCallbackFunction([scene, overlays](sf::RenderWindow * window, InputState & state)
	{
		RemoveAllObjects();
		game_mode = PLAYING;
		LockMouse(*window);
		scene->ResetLevel();
		scene->SetExposure(1.0f);
		overlays->sound_click.play();
	}, true);
	rstbtn.AddObject(&button2, Object::Allign::CENTER);
	pausemenu.AddObject(&rstbtn, Object::Allign::LEFT);


	Box editbtn(600, 50);
	Text button_e(LOCAL["Edit"], LOCAL("default"), 40, sf::Color::White);
	button_e.SetBorderColor(sf::Color::Black);
	button_e.SetBorderWidth(2);
	editbtn.SetBackgroundColor(sf::Color(200, 200, 200, 200));
	editbtn.hoverstate.color_main = sf::Color(200, 40, 0, 255);
	editbtn.SetCallbackFunction([scene, overlays](sf::RenderWindow * window, InputState & state)
	{
		OpenEditor(scene, overlays, scene->level_copy.level_id);
		overlays->sound_click.play();
	}, true);
	editbtn.AddObject(&button_e, Object::Allign::CENTER);
	pausemenu.AddObject(&editbtn, Object::Allign::LEFT);

	Box screenshotbtn(600, 50);
	Text button_s(LOCAL["Take screenshot"], LOCAL("default"), 40, sf::Color::White);
	button_s.SetBorderColor(sf::Color::Black);
	button_s.SetBorderWidth(2);
	screenshotbtn.SetBackgroundColor(sf::Color(200, 200, 200, 200));
	screenshotbtn.hoverstate.color_main = sf::Color(200, 40, 0, 255);
	screenshotbtn.SetCallbackFunction([scene, overlays](sf::RenderWindow * window, InputState & state)
	{
		TakeScreenshot();
	}, true);
	screenshotbtn.AddObject(&button_s, Object::Allign::CENTER);
	pausemenu.AddObject(&screenshotbtn, Object::Allign::LEFT);
	
	Box sttbtn(600, 50);
	Text buttonstt(LOCAL["Settings"], LOCAL("default"), 40, sf::Color::White);
	buttonstt.SetBorderColor(sf::Color::Black);
	buttonstt.SetBorderWidth(2);
	sttbtn.SetBackgroundColor(sf::Color(200, 200, 200, 200));
	sttbtn.hoverstate.color_main = sf::Color(200, 40, 0, 255);
	sttbtn.SetCallbackFunction([scene, overlays](sf::RenderWindow * window, InputState & state)
	{
		overlays->TWBAR_ENABLED = !overlays->TWBAR_ENABLED;
		TwDefine("Settings iconified=false");
		overlays->sound_click.play();
	}, true);
	sttbtn.AddObject(&buttonstt, Object::Allign::CENTER);
	pausemenu.AddObject(&sttbtn, Object::Allign::LEFT);

	
	Box exitbtn(600, 50);
	Text button5(LOCAL["Quit"], LOCAL("default"), 40, sf::Color::White);
	button5.SetBorderColor(sf::Color::Black);
	button5.SetBorderWidth(2);
	exitbtn.SetBackgroundColor(sf::Color(200, 200, 200, 200));
	exitbtn.hoverstate.color_main = sf::Color(200, 40, 0, 255);
	exitbtn.SetCallbackFunction([scene, overlays](sf::RenderWindow * window, InputState & state)
	{
		RemoveAllObjects();
		if (scene->IsSinglePlay())
		{
			OpenLevelMenu(scene, overlays);
		}
		else
		{
			OpenMainMenu(scene, overlays);
		}
		scene->SetMode(Scene::INTRO);
	}, true);
	exitbtn.AddObject(&button5, Object::Allign::CENTER);
	pausemenu.AddObject(&exitbtn, Object::Allign::LEFT);

	AddGlobalObject(pausemenu);
}

void PauseGame(sf::RenderWindow& window, Overlays * overlays, Scene * scene) {
	game_mode = PAUSED;
	UnlockMouse(window);
	OpenPauseMenu(scene, overlays);
	scene->SetExposure(0.5f);
}

void OpenScreenSaver(Scene * scene, Overlays * overlays)
{
	RemoveAllObjects();
	game_mode = SCREEN_SAVER;
	scene->SetMode(Scene::SCREEN_SAVER);
}

void PlayNewGame(Scene * scene, sf::RenderWindow * window, int level)
{
	StopReplay();
	RemoveAllObjects();
	game_mode = PLAYING;
	scene->StartNewGame();
	LockMouse(*window);
}

void OpenTestWindow()
{
	Window test(200, 200, 500, 500, sf::Color(0, 0, 0, 128), LOCAL["Window"], LOCAL("default"));
	Text button(LOCAL["Button"], LOCAL("default"), 30, sf::Color::White);
	Box sbox(0, 0, 420, 200, sf::Color(128, 128, 128, 240));
	Box sbox2(0, 0, 240, 40, sf::Color(0, 64, 128, 240));
	Box sbox3(0, 0, 30, 30, sf::Color(0, 64, 128, 240));

	sbox2.hoverstate.color_main = sf::Color(230, 40, 20, 200);
	sbox2.AddObject(&button, Box::CENTER);
	button.hoverstate.font_size = 40;
	test.Add(&sbox, Box::CENTER);
	test.Add(&sbox2, Box::CENTER);
	test.Add(&sbox, Box::CENTER);
	test.Add(&sbox2, Box::CENTER);
	test.Add(&sbox, Box::CENTER);
	test.Add(&sbox2, Box::CENTER);
	
	AddGlobalObject(test);
}

void OpenLevelMenu(Scene* scene, Overlays* overlays)
{
	RemoveAllObjects();

	scene->SetCurrentMusic(scene->levels.GetMusic("menu.ogg"));

	sf::Vector2f wsize = default_size;
	sf::Vector2f vsize = default_view.getSize();
	MenuBox levels(wsize.x*0.95f, vsize.y*0.95f, false, (vsize.x - wsize.x*0.95f)/2, vsize.y*0.025f);
	levels.SetBackgroundColor(sf::Color(32,32,32,160));
	//make the menu static
	levels.static_object = true;

	scene->SetExposure(0.7f);
	scene->SetMode(Scene::INTRO);
	game_mode = LEVELS;

	Box lvlmargin(50, 4);
	lvlmargin.SetBackgroundColor(sf::Color::Transparent);

	std::map<int, std::string> names = scene->levels.getLevelNames();
	std::map<int, std::string> desc = scene->levels.getLevelDesc();
	std::vector<int> ids = scene->levels.getLevelIds();
	std::map<int, Score> scores = scene->levels.getLevelScores();
	Text lvl(LOCAL["Levels"], LOCAL("default"), 60, sf::Color::White);
	levels.AddObject(&lvl, Object::Allign::CENTER);
	levels.AddObject(&lvlmargin, Object::Allign::CENTER);

	Button Bk2Menu(LOCAL["Back2Main"], 600, 50,
		[scene, overlays](sf::RenderWindow * window, InputState & state)
		{
			OpenMainMenu(scene, overlays);
			overlays->sound_click.play();
		},
		sf::Color(200, 40, 0, 255), sf::Color(128, 128, 128, 128));
	levels.AddObject(&Bk2Menu, Object::Allign::LEFT);

	Box Newlvl(600, 50);
	Newlvl.SetBackgroundColor(sf::Color(128, 128, 128, 128));
	Text newlvl(LOCAL["CreateNewLvl"], LOCAL("default"), 40, sf::Color::White);
	Newlvl.hoverstate.color_main = sf::Color(200, 40, 0, 255);
	Newlvl.SetCallbackFunction([scene, overlays](sf::RenderWindow * window, InputState & state)
	{
		OpenEditor(scene, overlays, -1);
		overlays->sound_click.play();
	});
	Newlvl.AddObject(&newlvl, Object::Allign::CENTER);
	levels.AddObject(&Newlvl, Object::Allign::LEFT);
	
	sf::Image edit; edit.loadFromFile(edit_png);
	sf::Texture edittxt; edittxt.loadFromImage(edit);
	edittxt.setSmooth(true);

	sf::Image remove; remove.loadFromFile(delete_png);
	sf::Texture removetxt; removetxt.loadFromImage(remove);
	removetxt.setSmooth(true);

	for (int i = 0; i < scene->levels.GetLevelNum(); i++)
	{
		Box lvlbtton(wsize.x*0.95f - 60, 65);
		lvlbtton.SetBackgroundColor(sf::Color(128, 128, 128, 128));
		lvlbtton.hoverstate.border_thickness = 3;

		Box lvltext(500, 63);
		lvltext.SetBackgroundColor(sf::Color::Transparent);
		lvltext.SetMargin(0);
		Box lvltitle(500, 40);
		lvltitle.SetBackgroundColor(sf::Color::Transparent);
		Text lvlname(utf8_to_wstring(names[ids[i]]), LOCAL("default"), 30, sf::Color::White);
		Text lvldescr(utf8_to_wstring(desc[ids[i]]), LOCAL("default"), 15, sf::Color::White);
		lvlname.hoverstate.color_main = sf::Color(255, 0, 0, 255);
		lvlname.SetCallbackFunction([scene, overlays, selected = ids[i]](sf::RenderWindow * window, InputState & state)
		{
			PlayLevel(scene, window, selected);
			overlays->sound_click.play();
		});
		lvltitle.AddObject(&lvlname, Object::Allign::LEFT);
		lvltext.AddObject(&lvltitle, Object::Allign::LEFT);
	
		lvltext.AddObject(&lvlmargin, Object::Allign::CENTER);
		lvltext.AddObject(&lvldescr, Object::Allign::LEFT);
		lvlbtton.AddObject(&lvltext, Object::Allign::LEFT);

		Box lvlscore(500, 62);
		lvlscore.SetBackgroundColor(sf::Color::Transparent);
		std::string score_text = "--:--:--";
		if (scores[ids[i]].best_time != 0)
		{
			float time = scores[ids[i]].best_time;
			float minutes = floor(time / 60.f);
			float seconds = floor(time) - minutes*60;
			float mili = floor(time*100) - seconds*100 - minutes*6000;
			//convrt mili to frames
			score_text = num2str(minutes) + ":" + num2str(seconds) + ":" + num2str(round(mili*0.6f));
		}
		Text lvlscorev(score_text, LOCAL("default"), 35, sf::Color::White);
		lvlscorev.SetBackgroundColor(sf::Color::Green);
		lvlscore.AddObject(&lvlscorev, Object::Allign::CENTER);
		if (scores[ids[i]].best_time != 0 && fs::exists(std::string(recordings_folder) + "/" + ConvertSpaces2_(names[ids[i]]) + ".bin"))
		{
			lvlscore.AddObject(new Box(0, 5), Object::Allign::CENTER);
			lvlscore.AddObject(new Button(LOCAL["Replay best run"], 260, 20,
				[scene, overlays, id = ids[i]](sf::RenderWindow * window, InputState & state)
			{
				RePlayBest(scene, window, id);
				overlays->sound_click.play();
			},
				sf::Color(200, 40, 0, 255), sf::Color(128, 128, 128, 128)), Object::Allign::CENTER);
		}
		lvlbtton.AddObject(&lvlscore, Object::Allign::LEFT);

		Box lvlavg(500, 40);
		lvlavg.SetBackgroundColor(sf::Color::Transparent);
		score_text = "--:--:--";
		if (scores[ids[i]].best_time != 0)
		{
			float time = scores[ids[i]].all_time/scores[ids[i]].played_num;
			float minutes = floor(time / 60.f);
			float seconds = floor(time) - minutes * 60;
			float mili = floor(time * 100) - seconds * 100 - minutes * 6000;
			score_text = num2str(minutes) + ":" + num2str(seconds) + ":" + num2str(round(mili*0.6f));
		}
		Text lvlavgtxt(score_text, LOCAL("default"), 35, sf::Color::White);
		lvlavgtxt.SetBackgroundColor(sf::Color::White);
		lvlavg.AddObject(&lvlavgtxt, Object::Allign::CENTER);
		lvlbtton.AddObject(&lvlavg, Object::Allign::LEFT);

		Box buttons(120, 60);
		buttons.SetBackgroundColor(sf::Color::Transparent);
		Box bedit(56, 56);
		bedit.defaultstate.color_main = sf::Color(255, 255, 255, 255);
		bedit.hoverstate.color_main = sf::Color(0, 255, 0, 255);
		bedit.SetBackground(edittxt);
		bedit.SetCallbackFunction([scene, overlays, id = ids[i]](sf::RenderWindow * window, InputState & state)
		{
			OpenEditor(scene, overlays, id);
			overlays->sound_click.play();
		}, true);

		Box bremove(56, 56);
		bremove.defaultstate.color_main = sf::Color(255, 255, 255, 255);
		bremove.hoverstate.color_main = sf::Color(255, 0, 0, 255);
		bremove.SetBackground(removetxt);

		bremove.SetCallbackFunction([scene, overlays, id = ids[i]](sf::RenderWindow * window, InputState & state)
		{
			ConfirmLevelDeletion(id, scene, overlays);
			overlays->sound_click.play();
		}, true);

		buttons.AddObject(&bremove, Object::Allign::RIGHT);
		buttons.AddObject(&bedit, Object::Allign::RIGHT);
		lvlbtton.AddObject(&buttons, Object::Allign::RIGHT);
		levels.AddObject(&lvlbtton, Object::Allign::LEFT);
	}

	AddGlobalObject(levels);
}

void ConfirmLevelDeletion(int lvl, Scene* scene, Overlays* overlays)
{
	sf::Vector2f wsize = default_size;
	Window confirm(wsize.x*0.4f, wsize.y*0.4f, 500, 215, sf::Color(0, 0, 0, 128), LOCAL["You_sure"], LOCAL("default"));
	Text button1(LOCAL["Yes"], LOCAL("default"), 30, sf::Color::White);
	Text button2(LOCAL["No"], LOCAL("default"), 30, sf::Color::White);
	Text text(LOCAL["You_sure"], LOCAL("default"), 30, sf::Color::White);
	 
	Box but1(0, 0, 240, 40, sf::Color(0, 64, 128, 240));
	Box but2(0, 0, 240, 40, sf::Color(0, 64, 128, 240));

	but1.hoverstate.color_main = sf::Color(230, 40, 20, 200);
	but2.hoverstate.color_main = sf::Color(40, 230, 20, 200);
	but1.AddObject(&button1, Box::CENTER);
	but2.AddObject(&button2, Box::CENTER);

	confirm.Add(&text, Box::CENTER);
	confirm.Add(&but1, Box::RIGHT);
	confirm.Add(&but2, Box::RIGHT);

	int id = AddGlobalObject(confirm);

	get_glob_obj(id).objects[1].get()->objects[0].get()->objects[1].get()->SetCallbackFunction([scene, overlays, id, lvl](sf::RenderWindow * window, InputState & state)
	{
		//remove lvl
		scene->levels.DeleteLevel(lvl);
		overlays->sound_click.play();
		OpenLevelMenu(scene, overlays);
	}, true);

	get_glob_obj(id).objects[1].get()->objects[0].get()->objects[2].get()->SetCallbackFunction([scene, overlays, id](sf::RenderWindow * window, InputState & state)
	{
		Add2DeleteQueue(id);
		overlays->sound_click.play();
	});
}

void ConfirmEditorExit(Scene* scene, Overlays* overlays)
{
	sf::Vector2f wsize = default_size;
	Window confirm(wsize.x*0.4f, wsize.y*0.4f, 500, 215, sf::Color(0, 0, 0, 128), LOCAL["You_sure"], LOCAL("default"));
	Text button1(LOCAL["Yes"], LOCAL("default"), 30, sf::Color::White);
	Text button2(LOCAL["No"], LOCAL("default"), 30, sf::Color::White);
	Text text(LOCAL["You_sure"], LOCAL("default"), 30, sf::Color::White);

	Box but1(0, 0, 240, 40, sf::Color(0, 64, 128, 240));
	Box but2(0, 0, 240, 40, sf::Color(0, 64, 128, 240));

	but1.hoverstate.color_main = sf::Color(230, 40, 20, 200);
	but2.hoverstate.color_main = sf::Color(40, 230, 20, 200);
	but1.AddObject(&button1, Box::CENTER);
	but2.AddObject(&button2, Box::CENTER);

	confirm.Add(&text, Box::CENTER);
	confirm.Add(&but1, Box::RIGHT);
	confirm.Add(&but2, Box::RIGHT);

	int id = AddGlobalObject(confirm);

	get_glob_obj(id).objects[1].get()->objects[0].get()->objects[1].get()->SetCallbackFunction([scene, overlays, id](sf::RenderWindow * window, InputState & state)
	{
		OpenLevelMenu(scene, overlays);
		scene->ExitEditor();
		scene->StopAllMusic();
		overlays->TWBAR_ENABLED = false;
		TwDefine("LevelEditor visible=false");
		TwDefine("FractalEditor visible=false");
		overlays->sound_click.play();
	});

	get_glob_obj(id).objects[1].get()->objects[0].get()->objects[2].get()->SetCallbackFunction([scene, overlays, id](sf::RenderWindow * window, InputState & state)
	{
		Add2DeleteQueue(id);
		overlays->sound_click.play();
	});
}


void ConfirmExit(Scene* scene, Overlays* overlays)
{
	sf::Vector2f wsize = default_size;
	Window confirm(wsize.x*0.4f, wsize.y*0.4f, 500, 215, sf::Color(0, 0, 0, 128), LOCAL["You_sure"], LOCAL("default"));
	Text button1(LOCAL["Yes"], LOCAL("default"), 30, sf::Color::White);
	Text button2(LOCAL["No"], LOCAL("default"), 30, sf::Color::White);
	Text text(LOCAL["You_sure"], LOCAL("default"), 30, sf::Color::White);

	Box but1(0, 0, 240, 40, sf::Color(0, 64, 128, 240));
	Box but2(0, 0, 240, 40, sf::Color(0, 64, 128, 240));

	but1.hoverstate.color_main = sf::Color(230, 40, 20, 200);
	but2.hoverstate.color_main = sf::Color(40, 230, 20, 200);
	but1.AddObject(&button1, Box::CENTER);
	but2.AddObject(&button2, Box::CENTER);

	confirm.Add(&text, Box::CENTER);
	confirm.Add(&but1, Box::RIGHT);
	confirm.Add(&but2, Box::RIGHT);

	int id = AddGlobalObject(confirm);

	get_glob_obj(id).objects[1].get()->objects[0].get()->objects[1].get()->SetCallbackFunction([scene, overlays, id](sf::RenderWindow * window, InputState & state)
	{
		window->close();
		overlays->sound_click.play();
	});

	get_glob_obj(id).objects[1].get()->objects[0].get()->objects[2].get()->SetCallbackFunction([scene, overlays, id](sf::RenderWindow * window, InputState & state)
	{
		Add2DeleteQueue(id);
		overlays->sound_click.play();
	});
}


void DisplayError(std::string error_text)
{
	sf::Vector2f wsize = default_size;
	Window error_window(wsize.x*0.4f, wsize.y*0.4f, 400, 215, sf::Color(100, 0, 0, 128), LOCAL["Error"], LOCAL("default"));
	
	error_window.Add(new Text(error_text, LOCAL("default"), 30, sf::Color::Red), Object::Allign::CENTER);

	Scene* scene = scene_ptr;
	Overlays* overlays = overlays_ptr;

	error_window.Add(new Button(LOCAL["Ok"], 240, 40,
		[scene, overlays](sf::RenderWindow * window, InputState & state)
		{
			overlays->sound_click.play();
		},
		sf::Color(200, 40, 0, 255), sf::Color(128, 128, 128, 128)), Object::Allign::RIGHT);

	int id = AddGlobalObject(error_window);

	get_glob_obj(id).objects[1].get()->objects[0].get()->objects[1].get()->SetCallbackFunction(
	[scene, overlays, id](sf::RenderWindow * window, InputState & state)
	{
		Add2DeleteQueue(id);
	});
}

void LockMouse(sf::RenderWindow& window) {
	window.setMouseCursorVisible(false);
	sf::Mouse::setPosition(sf::Vector2i(window.getSize().x*0.5, window.getSize().y*0.5), window);
}

void UnlockMouse(sf::RenderWindow& window) {
	window.setMouseCursorVisible(true);
}

int DirExists(const char *path) {
	return (int)fs::exists(fs::path(path));
}

void FirstStart(Overlays* overlays)
{
	TwDefine("First_launch visible=true label='First Launch'");
	TwDefine("Statistics visible=false");
	TwDefine("Settings visible=false");
	game_mode = FIRST_START;
	overlays->TWBAR_ENABLED = true;

	RemoveAllObjects();

	sf::VideoMode fs_size = sf::VideoMode::getDesktopMode();
	int barPos[2] = { (fs_size.width - 600)/2 , (fs_size.height - 100.f - 200) / 2 };
	TwSetParam(overlays_ptr->flaunch, NULL, "position", TW_PARAM_INT32, 2, barPos); 

	sf::Vector2f wsize = default_size;
	sf::Vector2f vsize = default_view.getSize();
	MenuBox mainmenu(1000, vsize.y*0.95f, false, wsize.x*0.025, wsize.y*0.025f);
	mainmenu.SetBackgroundColor(sf::Color::Transparent);
	//make the menu static
	mainmenu.static_object = true;

	//TITLE
	Text ttl("Marble\nMarcher", LOCAL("default"), 120, sf::Color::White);
	ttl.SetBorderColor(sf::Color::Black);
	ttl.SetBorderWidth(4);
	mainmenu.AddObject(&ttl, Object::Allign::LEFT);

	Box margin1(800, 5);
	margin1.SetBackgroundColor(sf::Color::Transparent);
	mainmenu.AddObject(&margin1, Object::Allign::LEFT);

	Text CE("Community Edition", LOCAL("default"), 60, sf::Color::White);
	CE.SetBorderColor(sf::Color::Black);
	CE.SetBorderWidth(4);
	mainmenu.AddObject(&CE, Object::Allign::LEFT);

	AddGlobalObject(mainmenu);
}


//ANTTWEAKBAR stuff


sf::Vector2i getResolution(int i)
{
	switch (i)
	{
	case 0:
		return sf::Vector2i(320, 240);
	case 1:
		return sf::Vector2i(480, 320);
	case 2:
		return sf::Vector2i(640, 480);
	case 3:
		return sf::Vector2i(800, 480);
	case 4:
		return sf::Vector2i(960, 540);
	case 5:
		return sf::Vector2i(1136, 640);
	case 6:
		return sf::Vector2i(1280, 720);
	case 7:
		return sf::Vector2i(1600, 900);
	case 8:
		return sf::Vector2i(1920, 1080);
	case 9:
		return sf::Vector2i(2048, 1152);
	case 10:
		return sf::Vector2i(2560, 1440);
	case 11:
		return sf::Vector2i(3840, 2160);
	case 12:
		return sf::Vector2i(7680, 4320);
	case 13:
		return sf::Vector2i(10240, 4320);
	case 14:
		return sf::Vector2i(240, 140);
	}
}


void TakeScreenshot()
{
	taken_screenshot = true;
	sf::Vector2i rendering_resolution = getResolution(SETTINGS.stg.rendering_resolution);
	sf::Vector2i screenshot_resolution = getResolution(SETTINGS.stg.screenshot_resolution);

	scene_ptr->SetResolution(screenshot_resolution.x, screenshot_resolution.y);
	renderer_ptr->ReInitialize(screenshot_resolution.x, screenshot_resolution.y);

	scene_ptr->WriteRenderer(*renderer_ptr);
	renderer_ptr->SetOutputTexture(*screenshot_txt);

	renderer_ptr->camera.SetMotionBlur(0);
	renderer_ptr->Render();

	screenshot_txt->copyToImage().saveToFile((std::string)"screenshots/screenshot" + (std::string)num2str(time(NULL)) + ".jpg");

	scene_ptr->SetResolution(rendering_resolution.x, rendering_resolution.y);
	renderer_ptr->ReInitialize(rendering_resolution.x, rendering_resolution.y);
	renderer_ptr->SetOutputTexture(*main_txt);
	overlays_ptr->sound_screenshot.play();
	screenshot_clock.restart();
}

void InitializeRendering(std::string config)
{
	sf::Vector2i rendering_resolution = getResolution(SETTINGS.stg.rendering_resolution);
	sf::Vector2i screenshot_resolution = getResolution(SETTINGS.stg.screenshot_resolution);

	renderer_ptr->variables["MRRM_scale"] = SETTINGS.stg.MRRM_scale;
	renderer_ptr->variables["shadow_scale"] = SETTINGS.stg.shadow_resolution;
	renderer_ptr->variables["bloom_scale"] = SETTINGS.stg.bloom_resolution;
	scene_ptr->SetResolution(rendering_resolution.x, rendering_resolution.y);
	renderer_ptr->Initialize(rendering_resolution.x, rendering_resolution.y, renderer_ptr->GetConfigFolder() + "/" + config);
	
	renderer_ptr->camera.bloomintensity = SETTINGS.stg.bloom_intensity;
	renderer_ptr->camera.bloomradius = SETTINGS.stg.bloom_radius;
	renderer_ptr->camera.auto_exposure_speed = SETTINGS.stg.auto_exposure_speed;
	renderer_ptr->camera.auto_exposure_target = SETTINGS.stg.auto_exposure_target;
	renderer_ptr->camera.SetMotionBlur(SETTINGS.stg.motion_blur);
	renderer_ptr->camera.SetFOV(SETTINGS.stg.FOV);
	renderer_ptr->camera.cross_eye = SETTINGS.stg.cross_eye;
	renderer_ptr->camera.eye_separation = SETTINGS.stg.eye_separation;
	renderer_ptr->camera.SetExposure(SETTINGS.stg.exposure);

	scene_ptr->Refl_Refr_Enabled = SETTINGS.stg.refl_refr;
	scene_ptr->Shadows_Enabled = SETTINGS.stg.shadows;
	scene_ptr->Fog_Enabled = SETTINGS.stg.fog;
	scene_ptr->gamma_camera = SETTINGS.stg.gamma_camera;
	scene_ptr->gamma_material = SETTINGS.stg.gamma_material;
	scene_ptr->gamma_sky = SETTINGS.stg.gamma_sky;

	main_txt->create(rendering_resolution.x, rendering_resolution.y);
	renderer_ptr->SetOutputTexture(*main_txt);
	screenshot_txt->create(screenshot_resolution.x, screenshot_resolution.y);
}

int music_id = 0;
bool music_play = false;

void TW_CALL MarbleSet(void *data)
{
	scene_ptr->cur_ed_mode = Scene::EditorMode::PLACE_MARBLE;
}

void TW_CALL FlagSet(void *data)
{
	scene_ptr->cur_ed_mode = Scene::EditorMode::PLACE_FLAG;
}

void TW_CALL PlayMusic(void *data)
{
	scene_ptr->levels.StopAllMusic();
	music_play = !music_play;
	if (music_play)
	{
		scene_ptr->levels.GetMusicByID(music_id)->play();
	}
}

void TW_CALL SaveLevel(void *data)
{
	Level* copy = &scene_ptr->level_copy;
	int lvlid = scene_ptr->GetLevel();

	std::vector<std::string> music_list = scene_ptr->levels.GetMusicNames();
	std::vector<int> lvlnum = scene_ptr->levels.getLevelIds();
	copy->use_music = music_list[music_id];
	bool same_level = scene_ptr->original_level_name == copy->txt;
	if (lvlid < 0 || !same_level)
		lvlid = time(NULL);
	copy->level_id = lvlid;
	copy->SaveToFile(std::string(level_folder) + "/" + ConvertSpaces2_(copy->txt) + ".lvl", lvlid, copy->link_level);
	scene_ptr->levels.ReloadLevels();
	if (!(scene_ptr->GetLevel() >= 0 && same_level))
	{
		scene_ptr->WriteLVL(lvlid);
		scene_ptr->original_level_name = copy->txt;
	}
}

void TW_CALL PlayThisLevel(void *data)
{
	if (scene_ptr->levels.LevelExists(scene_ptr->GetLevel()))
	{
		scene_ptr->ExitEditor();
		TwDefine("LevelEditor visible=false");
		TwDefine("FractalEditor visible=false");
		PlayLevel(scene_ptr, window, scene_ptr->GetLevel());
	}
}



void TW_CALL CopyStdStringToClient(std::string& destinationClientString, const std::string& sourceLibraryString)
{
	// Copy the content of souceString handled by the AntTweakBar library to destinationClientString handled by your application
	destinationClientString = sourceLibraryString;
}


void TW_CALL ApplySettings(void *data)
{
	//if window is not yet created or when the fullscreen setting is changed
	if (!window->isOpen() || SETTINGS.stg.fullscreen != fullscreen_current)
	{
		fullscreen_current = SETTINGS.stg.fullscreen;

		sf::VideoMode screen_size;
		sf::Uint32 window_style;
		bool fullscreen = SETTINGS.stg.fullscreen;
		if (fullscreen) {
			screen_size = sf::VideoMode::getDesktopMode();
			window_style = sf::Style::Fullscreen;
		}
		else {
			screen_size = sf::VideoMode::getDesktopMode();
			window_style = sf::Style::Default;
		}

		//GL settings
		sf::ContextSettings settings;
		settings.majorVersion = 4;
		settings.minorVersion = 3;

		window->create(screen_size, "Marble Marcher: Community Edition", window_style, settings);
		window->setVerticalSyncEnabled(SETTINGS.stg.VSYNC);
		window->setKeyRepeatEnabled(false);
		
		INIT();

		if (!fullscreen)
		{
			sf::VideoMode fs_size = sf::VideoMode::getDesktopMode();
			window->setSize(sf::Vector2u(fs_size.width, fs_size.height - 100.f));
			window->setPosition(sf::Vector2i(0, 0));
		}

		SETTINGS.first_start = false;

		overlays_ptr->SetAntTweakBar(window->getSize().x, window->getSize().y);
	}

	window->setFramerateLimit(SETTINGS.stg.fps_limit);
	std::vector<std::string> langs = LOCAL.GetLanguages();
	LOCAL.SetLanguage(langs[SETTINGS.stg.language]);

	std::vector<std::string> configs = renderer_ptr->GetConfigurationsList();

	InitializeRendering(configs[SETTINGS.stg.shader_config]);

	if (current_music != nullptr)
		current_music->setVolume(SETTINGS.stg.music_volume);

	scene_ptr->sound_goal.setVolume(SETTINGS.stg.fx_volume);
	scene_ptr->sound_bounce1.setVolume(SETTINGS.stg.fx_volume);
	scene_ptr->sound_bounce2.setVolume(SETTINGS.stg.fx_volume);
	scene_ptr->sound_bounce3.setVolume(SETTINGS.stg.fx_volume);
	scene_ptr->sound_shatter.setVolume(SETTINGS.stg.fx_volume);
	scene_ptr->MarbleType = SETTINGS.stg.marble_type;
	scene_ptr->PlayNext = SETTINGS.stg.play_next;

	overlays_ptr->sound_hover.setVolume(SETTINGS.stg.fx_volume);
	overlays_ptr->sound_click.setVolume(SETTINGS.stg.fx_volume);
	overlays_ptr->sound_count.setVolume(SETTINGS.stg.fx_volume);
	overlays_ptr->sound_go.setVolume(SETTINGS.stg.fx_volume);
	overlays_ptr->sound_screenshot.setVolume(SETTINGS.stg.fx_volume);
}

void TW_CALL InitialOK(void *data)
{
	ApplySettings(nullptr);
	TwDefine("First_launch visible=false");
	TwDefine("Statistics visible=true");
	TwDefine("Settings visible=true");
	overlays_ptr->TWBAR_ENABLED = false;
	OpenMainMenu(scene_ptr, overlays_ptr);
}

void TW_CALL ResetEditorCamera(void* data)
{
	scene_ptr->ResetCamera();
}

void TW_CALL StartRecording(void* data)
{
	if (!recording)
	{
		StartRecording();
	}
	else
	{
		StopRecording2File("input_recording.bin");
	}
}

void TW_CALL StartReplay(void* data)
{
	if (!replay)
	{
		StartReplayFromFile("input_recording.bin");
	}
	else
	{
		StopReplay();
	}
}


#if(DEBUG_MODE)
	TwBar *debug;
#endif

void InitializeATBWindows(float* fps, float *target_fps)
{
	overlays_ptr->stats = TwNewBar("Statistics");
	TwDefine(" GLOBAL help='Marble Marcher: Community Edition. \nUse F4 to open or close settings windows. \nCtr+D to enable the debug menu' ");

	std::map<int, std::string> level_list = scene_ptr->levels.getLevelNames();
	std::vector<int> lvlnum = scene_ptr->levels.getLevelIds();
	TwEnumVal *level_enums = new TwEnumVal[level_list.size() + 1];
	TwEnumVal enumval;
	enumval.Label = "None";
	enumval.Value = -1;
	level_enums[0] = enumval;
	int i = 1;
	for (auto &lvlID : lvlnum)
	{
		enumval.Label = level_list[lvlID].c_str();
		enumval.Value = lvlID;
		level_enums[i++] = enumval;
	}

	TwType Levels = TwDefineEnum("levels", level_enums, level_list.size() + 1);

	TwType Resolutions = TwDefineEnum("Resolutions", resolutions, 14);

	std::vector<std::string> music_list = scene_ptr->levels.GetMusicNames();
	TwEnumVal *music_enums = new TwEnumVal[music_list.size()];
	for (int i = 0; i < music_list.size(); i++)
	{
		TwEnumVal enumval;
		enumval.Label = music_list[i].c_str();
		enumval.Value = i;
		music_enums[i] = enumval;
	}

	TwType Level_music = TwDefineEnum("Level music", music_enums, music_list.size());

	std::vector<std::string> langs = LOCAL.GetLanguages();
	TwEnumVal *language_enums = new TwEnumVal[langs.size()];

	for (int j = 0; j < langs.size(); j++)
	{
		enumval.Label = langs[j].c_str();
		enumval.Value = j;
		language_enums[j] = enumval;
	}

	TwType Languages = TwDefineEnum("Languages", language_enums, langs.size());

	std::vector<std::string> configs = renderer_ptr->GetConfigurationsList();
	TwEnumVal *config_enums = new TwEnumVal[configs.size()];

	for (int j = 0; j < configs.size(); j++)
	{
		enumval.Label = configs[j].c_str();
		enumval.Value = j;
		config_enums[j] = enumval;
	}

	TwType Configurations = TwDefineEnum("Configurations", config_enums, configs.size());

	// Change bar position
	int barPos[2] = { 16, 60 };
	TwSetParam(overlays_ptr->stats, NULL, "position", TW_PARAM_INT32, 2, &barPos);
	TwAddVarRO(overlays_ptr->stats, "FPS", TW_TYPE_FLOAT, fps, " label='FPS' ");
	TwAddVarRO(overlays_ptr->stats, "Marble velocity", TW_TYPE_DIR3F, scene_ptr->marble_vel.data(), " ");
	TwAddVarRO(overlays_ptr->stats, "Marble position", TW_TYPE_DIR3F, scene_ptr->marble_pos.data(), " ");

	overlays_ptr->settings = TwNewBar("Settings");

	TwAddVarRW(overlays_ptr->settings, "Rendering resolution", Resolutions, &SETTINGS.stg.rendering_resolution, "group='Rendering settings'");
	TwAddVarRW(overlays_ptr->settings, "Fullscreen", TW_TYPE_BOOLCPP, &SETTINGS.stg.fullscreen, "group='Rendering settings' help='You need to restart the game for changes to take effect'");
	TwAddVarRW(overlays_ptr->settings, "Framerate limit", TW_TYPE_INT32, &SETTINGS.stg.fps_limit, "min=1 max=240 group='Rendering settings'");
	TwAddVarRW(overlays_ptr->settings, "Cross-eye 3D", TW_TYPE_BOOLCPP, &SETTINGS.stg.cross_eye, "group='Rendering settings'");
	TwAddVarRW(overlays_ptr->settings, "3D eye separation", TW_TYPE_FLOAT, &SETTINGS.stg.eye_separation, "min=-2 step=0.05 max=2 group='Rendering settings'");
	TwAddVarRW(overlays_ptr->settings, "Screenshot resolution", Resolutions, &SETTINGS.stg.screenshot_resolution, "group='Rendering settings'");
	TwAddVarRW(overlays_ptr->settings, "Screenshot preview", TW_TYPE_BOOLCPP, &SETTINGS.stg.screenshot_preview, "group='Rendering settings'");
	TwAddVarRW(overlays_ptr->settings, "Screenshot preview time", TW_TYPE_FLOAT, &SETTINGS.stg.preview_time, "min=0 step=0.01 max=5 group='Rendering settings'");
	TwAddVarRW(overlays_ptr->settings, "Shader configuration", Configurations, &SETTINGS.stg.shader_config, "group='Rendering settings'");
	TwAddVarRW(overlays_ptr->settings, "Multi Resolution Ray Marching scaling", TW_TYPE_INT32, &SETTINGS.stg.MRRM_scale, "min=2 max=8 group='Rendering settings' help='Don't touch this'");
	TwAddVarRW(overlays_ptr->settings, "Shadow downscaling", TW_TYPE_INT32, &SETTINGS.stg.shadow_resolution, "min=1 max=8 group='Rendering settings'");
	TwAddVarRW(overlays_ptr->settings, "Bloom downscaling", TW_TYPE_INT32, &SETTINGS.stg.bloom_resolution, "min=1 max=8 group='Rendering settings'");
	TwAddButton(overlays_ptr->settings, "Apply1", ApplySettings, NULL, "group='Rendering settings' label='Apply settings'  ");

	TwAddVarRW(overlays_ptr->settings, "Auto-exposure speed", TW_TYPE_FLOAT, &SETTINGS.stg.auto_exposure_speed, "min=0 step=0.01 max=1 group='Graphics settings' help='The speed at which the camera adapts to the scene brightness' ");
	TwAddVarRW(overlays_ptr->settings, "Auto-exposure target brightness", TW_TYPE_FLOAT, &SETTINGS.stg.auto_exposure_target, "min=0 step=0.01 max=1 group='Graphics settings'");
	TwAddVarRW(overlays_ptr->settings, "FOV", TW_TYPE_FLOAT, &SETTINGS.stg.FOV, "min=30 step=1 max=180 group='Graphics settings'");
	TwAddVarRW(overlays_ptr->settings, "VSYNC", TW_TYPE_BOOLCPP, &SETTINGS.stg.VSYNC, "group='Graphics settings'");
	TwAddVarRW(overlays_ptr->settings, "Shadows", TW_TYPE_BOOLCPP, &SETTINGS.stg.shadows, "group='Graphics settings'");
	TwAddVarRW(overlays_ptr->settings, "Reflection and Refraction", TW_TYPE_BOOLCPP, &SETTINGS.stg.refl_refr, "group='Graphics settings'");
	TwAddVarRW(overlays_ptr->settings, "Volumetric fog(TODO)", TW_TYPE_BOOLCPP, &SETTINGS.stg.fog, "group='Graphics settings'");
	TwAddVarRW(overlays_ptr->settings, "Blur", TW_TYPE_FLOAT, &SETTINGS.stg.motion_blur, "min=0 step=0.001 max=0.75 group='Graphics settings'");
	TwAddVarRW(overlays_ptr->settings, "Exposure", TW_TYPE_FLOAT, &SETTINGS.stg.exposure, "min=0 max=5 step=0.001 group='Graphics settings'");
	TwAddVarRW(overlays_ptr->settings, "Bloom Intensity", TW_TYPE_FLOAT, &SETTINGS.stg.bloom_intensity, "min=0 max=5 step=0.001 group='Graphics settings'");
	TwAddVarRW(overlays_ptr->settings, "Bloom Radius", TW_TYPE_FLOAT, &SETTINGS.stg.bloom_radius, "min=1 max=10 step=0.1 group='Graphics settings'");
	TwAddVarRW(overlays_ptr->settings, "Material gamma", TW_TYPE_FLOAT, &SETTINGS.stg.gamma_material, "min=0 max=4 step=0.1 group='Graphics settings'");
	TwAddVarRW(overlays_ptr->settings, "Sky gamma", TW_TYPE_FLOAT, &SETTINGS.stg.gamma_sky, "min=0 max=4 step=0.1 group='Graphics settings'");
	TwAddVarRW(overlays_ptr->settings, "Camera gamma", TW_TYPE_FLOAT, &SETTINGS.stg.gamma_camera, "min=0 max=4 step=0.1 group='Graphics settings'");
	TwAddButton(overlays_ptr->settings, "Apply2", ApplySettings, NULL, "group='Graphics settings' label='Apply settings'  ");

	TwAddVarRW(overlays_ptr->settings, "Language", Languages, &SETTINGS.stg.language, "group='Gameplay settings'");
	TwEnumVal marble_type[] = { { 0,  "Glass"  },
								{ 1,  "Metal" },
								{ 2,  "Ceramic" } };

	TwType Marble_type = TwDefineEnum("Marble type", marble_type, 3);
	TwAddVarRW(overlays_ptr->settings, "Marble type", Marble_type, &SETTINGS.stg.marble_type, "group='Gameplay settings'");
	TwAddVarRW(overlays_ptr->settings, "Play next level", TW_TYPE_BOOLCPP, &SETTINGS.stg.play_next, "group='Gameplay settings' help='Will play next level of a level pack if enabled'");
	TwAddVarRW(overlays_ptr->settings, "Music volume", TW_TYPE_FLOAT, &SETTINGS.stg.music_volume, "min=0 max=100 step=1 group='Gameplay settings'");
	TwAddVarRW(overlays_ptr->settings, "FX volume", TW_TYPE_FLOAT, &SETTINGS.stg.fx_volume, "min=0 max=100 step=1 group='Gameplay settings'");
	TwAddVarRW(overlays_ptr->settings, "Camera size", TW_TYPE_FLOAT, &scene_ptr->camera_size, "min=0 max=10 step=0.001 group='Gameplay settings'");
	TwAddVarRW(overlays_ptr->settings, "Camera speed(Free mode)", TW_TYPE_FLOAT, &scene_ptr->free_camera_speed, "min=0 max=100 step=0.001 group='Gameplay settings'");

	TwAddButton(overlays_ptr->settings, "Apply3", ApplySettings, NULL, "group='Gameplay settings' label='Apply settings'  ");

	TwAddVarRW(overlays_ptr->settings, "Mouse sensitivity", TW_TYPE_FLOAT, &SETTINGS.stg.mouse_sensitivity, "min=0.001 max=0.02 step=0.001 group='Control settings'");
	TwAddVarRW(overlays_ptr->settings, "Wheel sensitivity", TW_TYPE_FLOAT, &SETTINGS.stg.wheel_sensitivity, "min=0.01 max=0.5 step=0.01 group='Control settings'");
	TwAddVarRW(overlays_ptr->settings, "Gamepad deadzone", TW_TYPE_FLOAT, &SETTINGS.stg.gamepad_deadzone, "min=0.0 max=0.9 step=0.01 group='Control settings'");
	TwAddVarRW(overlays_ptr->settings, "Windows touch controls(experimental)", TW_TYPE_BOOLCPP, &SETTINGS.stg.touch_mode, "group='Control settings' help='Use a touchscreen'");
	TwAddButton(overlays_ptr->settings, "Apply4", ApplySettings, NULL, "group='Control settings' label='Apply settings'  ");
	int barPos1[2] = { 16, 250 };

	TwSetParam(overlays_ptr->settings, NULL, "position", TW_PARAM_INT32, 2, &barPos1);

	TwCopyStdStringToClientFunc(CopyStdStringToClient);

	overlays_ptr->level_editor = TwNewBar("LevelEditor");
	Level *copy = &scene_ptr->level_copy;

	TwAddVarRW(overlays_ptr->level_editor, "Level Name", TW_TYPE_STDSTRING, &copy->txt, "");
	TwAddVarRW(overlays_ptr->level_editor, "Level Description", TW_TYPE_STDSTRING, &copy->desc, "");

	TwAddButton(overlays_ptr->level_editor, "Save", SaveLevel, NULL,
		" label='Save Level'  ");

	TwAddButton(overlays_ptr->level_editor, "Play", PlayThisLevel, NULL,
		" label='Play (unsaved changes will be lost)'  ");

	TwAddButton(overlays_ptr->level_editor, "Set Marble", MarbleSet, NULL,
		" label='Set Marble Position'  help='Click on the fractal to place' ");

	TwAddButton(overlays_ptr->level_editor, "Set Flag", FlagSet, NULL,
		" label='Set Flag Position'  help='Click on the fractal to place' ");

	TwAddButton(overlays_ptr->level_editor, "Reset camera", ResetEditorCamera, NULL,
		" label='Reset camera' ");

	TwAddVarRW(overlays_ptr->level_editor, "Flag Position", TW_TYPE_DIR3F, copy->end_pos.data(), "");
	TwAddVarRW(overlays_ptr->level_editor, "Marble Position", TW_TYPE_DIR3F, copy->start_pos.data(), "");
	TwAddVarRW(overlays_ptr->level_editor, "Marble Radius(Scale)", TW_TYPE_FLOAT, &copy->marble_rad, "min=0 max=10 step=0.001 ");

	
	TwAddVarRW(overlays_ptr->level_editor, "Level music", Level_music, &music_id, "");

	TwAddButton(overlays_ptr->level_editor, "Play Music", PlayMusic, NULL, " label='Play/Stop current music'  ");

	
	TwAddVarRW(overlays_ptr->level_editor, "Play level after finish", Levels, &copy->link_level, "help = 'Which level is played after you finish this one, only works if the option Play Next Level is true'");

	TwAddVarRW(overlays_ptr->level_editor, "Sun direction", TW_TYPE_DIR3F, copy->light_dir.data(), "group='Level parameters'");
	TwAddVarRW(overlays_ptr->level_editor, "Sun color", TW_TYPE_DIR3F, copy->light_col.data(), "group='Level parameters'");
	//TwAddVarRW(level_editor, "Background color", TW_TYPE_DIR3F, copy->background_col.data(), "group='Level parameters'");
	TwAddVarRW(overlays_ptr->level_editor, "Gravity strength", TW_TYPE_FLOAT, &copy->gravity, "min=-0.5 max=0.5 step=0.0001 group='Level parameters'");
	TwAddVarRW(overlays_ptr->level_editor, "Kill y position (restart level)", TW_TYPE_FLOAT, &copy->kill_y, "min=-100 max=100 step=0.1 group='Level parameters'");
	TwAddVarRW(overlays_ptr->level_editor, "Is planet", TW_TYPE_BOOLCPP, &copy->planet, "group='Level parameters'");
	TwAddVarRW(overlays_ptr->level_editor, "Start look direction angle", TW_TYPE_FLOAT, &copy->start_look_x, "min=-3.14159 max=3.14159 step=0.01 group='Level parameters'");

	overlays_ptr->fractal_editor = TwNewBar("FractalEditor");

	TwAddVarRW(overlays_ptr->fractal_editor, "PBR roughness", TW_TYPE_FLOAT, &copy->PBR_roughness, "min=0 max=1 step=0.001 group='Fractal Material'");
	TwAddVarRW(overlays_ptr->fractal_editor, "PBR metallic", TW_TYPE_FLOAT, &copy->PBR_metal, "min=0 max=1 step=0.001 group='Fractal Material'");
	float *p = copy->params.data();
	TwAddVarRW(overlays_ptr->fractal_editor, "Fractal Iterations", TW_TYPE_INT32, &copy->FractalIter, "min=1 max=32 step=1 group='Fractal Coefficients'");
	TwAddVarRW(overlays_ptr->fractal_editor, "Fractal Scale", TW_TYPE_FLOAT, p, "min=0 max=5 step=0.0001 group='Fractal Coefficients'");
	TwAddVarRW(overlays_ptr->fractal_editor, "Fractal Angle1", TW_TYPE_FLOAT, p + 1, "min=-10 max=10 step=0.0001 group='Fractal Coefficients'");
	TwAddVarRW(overlays_ptr->fractal_editor, "Fractal Angle2", TW_TYPE_FLOAT, p + 2, "min=-10 max=10 step=0.0001  group='Fractal Coefficients'");
	TwAddVarRW(overlays_ptr->fractal_editor, "Fractal Shift", TW_TYPE_DIR3F, p + 3, "group='Fractal Coefficients'");
	TwAddVarRW(overlays_ptr->fractal_editor, "Fractal Color", TW_TYPE_DIR3F, p + 6, "group='Fractal Coefficients'");

	TwAddVarRW(overlays_ptr->fractal_editor, "Fractal Animation1", TW_TYPE_FLOAT, &copy->anim_1, "min=0 max=0.5 step=0.0001 group='Fractal Animation'");
	TwAddVarRW(overlays_ptr->fractal_editor, "Fractal Animation2", TW_TYPE_FLOAT, &copy->anim_2, "min=0 max=0.5 step=0.0001 group='Fractal Animation'");
	TwAddVarRW(overlays_ptr->fractal_editor, "Fractal Animation3", TW_TYPE_FLOAT, &copy->anim_3, "min=0 max=0.5 step=0.0001 group='Fractal Animation'");

	overlays_ptr->flaunch = TwNewBar("First_launch");
	 
	TwAddVarRW(overlays_ptr->flaunch, "Rendering resolution", Resolutions, &SETTINGS.stg.rendering_resolution, "");
	TwAddVarRW(overlays_ptr->flaunch, "Fullscreen", TW_TYPE_BOOLCPP, &SETTINGS.stg.fullscreen, " help='You need to restart the game for changes to take effect'");
	TwAddVarRW(overlays_ptr->flaunch, "Screenshot resolution", Resolutions, &SETTINGS.stg.screenshot_resolution, "");
	TwAddVarRW(overlays_ptr->flaunch, "Language", Languages, &SETTINGS.stg.language, "");
	TwAddButton(overlays_ptr->flaunch, "OK", InitialOK, NULL, " label='OK'  ");

	debug = TwNewBar("Debug_bar");
	TwAddButton(debug, "Record", StartRecording, NULL, " label='Record game input'  ");
	TwAddButton(debug, "Playback", StartReplay, NULL, " label='Play recording'  ");
	TwAddVarRW(debug, "Slow", TW_TYPE_BOOLCPP, &SETTINGS.stg.speed_regulation, " label='Speed regulation'");
	TwAddVarRW(debug, "Target FPS", TW_TYPE_FLOAT, target_fps, "min=1 max=240 step=1");
	TwAddVarRO(debug, "Replay frame", TW_TYPE_INT32, GetReplayFrame(), "");
	TwAddVarRO(debug, "Replay mode", TW_TYPE_BOOLCPP, &replay, "");
	TwAddVarRO(debug, "Recording mode", TW_TYPE_BOOLCPP, &recording, "");

	for (int i = 0; i < sf::Joystick::AxisCount; i++)
	{
		TwAddVarRO(debug, ("axis" + num2str(i)).c_str(), TW_TYPE_FLOAT, &io_state.axis_value[i], "group='Gamepad state'");
	}

	TwDefine("Debug_bar visible=false color='0 0 255' alpha=240 size='420 160' valueswidth=200");

	overlays_ptr->cheats = TwNewBar("Cheats");

	TwDefine("Cheats visible=false color='0 0 0' alpha=255 size='500 200' valueswidth=300");
	TwDefine(" GLOBAL fontsize=3 ");
	TwDefine("LevelEditor visible=false size='420 350' color='0 80 230' alpha=210 label='Level editor' valueswidth=200");
	TwDefine("FractalEditor visible=false size='420 350' color='0 120 200' alpha=210 label='Fractal editor' valueswidth=200");
	TwDefine("Settings color='255 128 0' alpha=210 size='420 500' valueswidth=100");
	TwDefine("First_launch visible=false color='0 0 0' alpha=255 size='500 200' valueswidth=300");
	TwDefine("Statistics color='0 128 255' alpha=210 size='420 160' valueswidth=200");
}
