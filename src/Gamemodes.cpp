#include "Gamemodes.h"

//Global variables
sf::Vector2i mouse_pos, mouse_prev_pos;
InputState io_state;

bool all_keys[sf::Keyboard::KeyCount] = { 0 };
bool mouse_clicked = false;
bool show_cheats = false;

//Constants
float mouse_sensitivity = 0.005f;
float wheel_sensitivity = 0.2f;
float music_vol = 75.0f;
float target_fps = 60.0f;

GameMode game_mode = MAIN_MENU;

void OpenMainMenu(Scene * scene, Overlays * overlays)
{
	RemoveAllObjects();
	game_mode = MAIN_MENU;
	scene->SetExposure(1.0f);
	scene->SetMode(Scene::INTRO);
	sf::Vector2f wsize = default_size;
	sf::Vector2f vsize = default_view.getSize();
	MenuBox mainmenu(1000, vsize.y*0.95f, wsize.x*0.025, wsize.y*0.025f);
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
	Text button5(LOCAL["Exit"], LOCAL("default"), 40, sf::Color::White);
	button5.SetBorderColor(sf::Color::Black);
	exitbtn.hoverstate.color_main = sf::Color(200, 40, 0, 255);
	exitbtn.SetCallbackFunction([scene, overlays](sf::RenderWindow * window, InputState & state)
	{
		overlays->sound_click.play();
		window->close();
	}, true);
	exitbtn.AddObject(&button5, Object::Allign::CENTER);
	mainmenu.AddObject(&exitbtn, Object::Allign::LEFT);

	Text about(LOCAL["About"], LOCAL("mono"), 30, sf::Color::White);
	about.SetBorderColor(sf::Color::Black);
	about.SetBorderWidth(3);
	mainmenu.AddObject(&about, Object::Allign::LEFT);
	

	AddGlobalObject(mainmenu);
}

void OpenEditor(Scene * scene, Overlays * overlays, int level)
{
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
	RemoveAllObjects();
	//play level
	game_mode = PLAYING;
	scene->SetExposure(1.0f);
	scene->levels.GetLevelMusic(level)->setVolume(GetVol());
	scene->levels.GetLevelMusic(level)->play();
	scene->StartSingle(level);
	LockMouse(*window);
}


void OpenControlMenu(Scene * scene, Overlays * overlays)
{
	RemoveAllObjects();
	game_mode = CONTROLS;
}




void OpenPauseMenu(Scene * scene, Overlays * overlays)
{
	RemoveAllObjects();
	scene->SetExposure(1.0f);
	sf::Vector2f wsize = default_size;
	sf::Vector2f vsize = default_view.getSize();
	MenuBox pausemenu(625, 510, wsize.x*0.025, wsize.y*0.025f);
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

	//PLAY
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
		scene->GetCurMusic().setVolume(GetVol());
		scene->SetExposure(1.0f);
		LockMouse(*window);
		overlays->sound_click.play();
	}, true);
	resumebtn.AddObject(&button1, Object::Allign::CENTER);
	pausemenu.AddObject(&resumebtn, Object::Allign::LEFT);

	//LEVELS
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
		scene->ResetLevel();
		scene->GetCurMusic().setVolume(GetVol());
		scene->SetExposure(1.0f);
		LockMouse(*window);
		overlays->sound_click.play();
	}, true);
	rstbtn.AddObject(&button2, Object::Allign::CENTER);
	pausemenu.AddObject(&rstbtn, Object::Allign::LEFT);

	//Settings
	Box sttbtn(600, 50);
	Text buttonstt(LOCAL["Settings"], LOCAL("default"), 40, sf::Color::White);
	buttonstt.SetBorderColor(sf::Color::Black);
	buttonstt.SetBorderWidth(2);
	sttbtn.SetBackgroundColor(sf::Color(200, 200, 200, 200));
	sttbtn.hoverstate.color_main = sf::Color(200, 40, 0, 255);
	sttbtn.SetCallbackFunction([scene, overlays](sf::RenderWindow * window, InputState & state)
	{
		overlays->TWBAR_ENABLED = !overlays->TWBAR_ENABLED;
		overlays->sound_click.play();
	}, true);
	sttbtn.AddObject(&buttonstt, Object::Allign::CENTER);
	pausemenu.AddObject(&sttbtn, Object::Allign::LEFT);

	//Exit
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
		scene->StopAllMusic();
	}, true);
	exitbtn.AddObject(&button5, Object::Allign::CENTER);
	pausemenu.AddObject(&exitbtn, Object::Allign::LEFT);

	AddGlobalObject(pausemenu);
}

void PauseGame(sf::RenderWindow& window, Overlays * overlays, Scene * scene) {
	game_mode = PAUSED;
	scene->GetCurMusic().setVolume(GetVol());
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
	RemoveAllObjects();
	game_mode = PLAYING;
	scene->StartNewGame();
	scene->GetCurMusic().setVolume(GetVol());
	scene->GetCurMusic().play();
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
	sf::Vector2f wsize = default_size;
	sf::Vector2f vsize = default_view.getSize();
	MenuBox levels(wsize.x*0.95f, vsize.y*0.95f, (vsize.x - wsize.x*0.95f)/2, vsize.y*0.025f);
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
	Box Bk2Menu(600, 50);
	Bk2Menu.SetBackgroundColor(sf::Color(128, 128, 128, 128));
	Text button(LOCAL["Back2Main"], LOCAL("default"), 40, sf::Color::White);
	Bk2Menu.hoverstate.color_main = sf::Color(200, 40, 0, 255);
	Bk2Menu.SetCallbackFunction([scene, overlays](sf::RenderWindow * window, InputState & state)
	{
		OpenMainMenu(scene, overlays);
		overlays->sound_click.play();
	});
	Bk2Menu.AddObject(&button, Object::Allign::CENTER);
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

		Box lvltext(500, 65);
		lvltext.SetBackgroundColor(sf::Color::Transparent);
		Box lvltitle(500, 40);
		lvltitle.SetBackgroundColor(sf::Color::Transparent);
		Text lvlname(utf8_to_wstring(names[ids[i]]), LOCAL("default"), 35, sf::Color::White);
		Text lvldescr(utf8_to_wstring(desc[ids[i]]), LOCAL("default"), 18, sf::Color::White);
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

		Box lvlscore(500, 40);
		lvlscore.SetBackgroundColor(sf::Color::Transparent);
		std::string score_text = "--:--:--";
		if (scores[ids[i]].best_time != 0)
		{
			float time = scores[ids[i]].best_time;
			float minutes = floor(time / 60.f);
			float seconds = floor(time) - minutes*60;
			float mili = floor(time*100) - seconds*100 - minutes*6000;
			//convrt mili to frames
			score_text = num2str(minutes) + ":" + num2str(seconds) + ":" + num2str(floor(mili*0.6f));
		}
		Text lvlscorev(score_text, LOCAL("default"), 35, sf::Color::White);
		lvlscorev.SetBackgroundColor(sf::Color::Green);
		lvlscore.AddObject(&lvlscorev, Object::Allign::CENTER);
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
			score_text = num2str(minutes) + ":" + num2str(seconds) + ":" + num2str(floor(mili*0.6f));
		}
		Text lvlavgtxt(score_text, LOCAL("default"), 35, sf::Color::White);
		lvlavgtxt.SetBackgroundColor(sf::Color::White);
		lvlavg.AddObject(&lvlavgtxt, Object::Allign::CENTER);
		lvlbtton.AddObject(&lvlavg, Object::Allign::LEFT);

		Box buttons(120, 60);
		buttons.SetBackgroundColor(sf::Color::Transparent);
		Box bedit(60, 60);
		bedit.defaultstate.color_main = sf::Color(255, 255, 255, 255);
		bedit.hoverstate.color_main = sf::Color(0, 255, 0, 255);
		bedit.SetBackground(edittxt);
		bedit.SetCallbackFunction([scene, overlays, id = ids[i]](sf::RenderWindow * window, InputState & state)
		{
			OpenEditor(scene, overlays, id);
			overlays->sound_click.play();
		}, true);

		Box bremove(60, 60);
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
	});

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


float GetVol() {
	if (game_settings.mute) {
		return 0.0f;
	}
	else if (game_mode == PAUSED) {
		return music_vol / 4;
	}
	else {
		return music_vol;
	}
}

void LockMouse(sf::RenderWindow& window) {
	window.setMouseCursorVisible(false);
	const sf::Vector2u size = window.getSize();
	mouse_pos = sf::Vector2i(size.x / 2, size.y / 2);
	sf::Mouse::setPosition(mouse_pos);
}
void UnlockMouse(sf::RenderWindow& window) {
	window.setMouseCursorVisible(true);
}


int DirExists(const char *path) {
	struct stat info;
	if (stat(path, &info) != 0) {
		return 0;
	}
	else if (info.st_mode & S_IFDIR) {
		return 1;
	}
	return 0;
}