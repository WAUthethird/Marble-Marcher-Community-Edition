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
#include "Overlays.h"
#include "Res.h"
#include <Localization.h>

static const float PI = 3.14159265359f;
static const int num_level_pages = 1 + (num_levels - 1) / Overlays::LEVELS_PER_PAGE;

Overlays::Overlays(Scene* scene) :
  draw_scale(1.0f),
  level_page(0),
  top_level(true),
  TWBAR_ENABLED(false)
{
  memset(all_hover, 0, sizeof(all_hover));
  buff_hover.loadFromFile(menu_hover_wav);
  sound_hover.setBuffer(buff_hover);
  buff_click.loadFromFile(menu_click_wav);
  sound_click.setBuffer(buff_click);
  buff_count.loadFromFile(count_down_wav);
  sound_count.setBuffer(buff_count);
  buff_go.loadFromFile(count_go_wav);
  sound_go.setBuffer(buff_go);

  buff_screenshot.loadFromFile(screenshot_wav);
  sound_screenshot.setBuffer(buff_screenshot);

  arrow_tex.loadFromFile(arrow_png);
  arrow_tex.setSmooth(true);
  arrow_spr.setTexture(arrow_tex);
  arrow_spr.setOrigin(arrow_spr.getLocalBounds().width / 2, arrow_spr.getLocalBounds().height / 2);

  ReloadLevelMenu(scene);
}


void Overlays::ReloadLevelMenu(Scene* scene)
{
	
}

Overlays::Texts Overlays::GetOption(Texts from, Texts to) {
  for (int i = from; i <= to; ++i) {
    if (all_hover[i]) {
      sound_click.play();
      return Texts(i);
    }
  }
  return Texts::TITLE;
}

void Overlays::UpdateMenu(float mouse_x, float mouse_y) {
  
}

void Overlays::UpdateControls(float mouse_x, float mouse_y) {

}

void Overlays::UpdateLevels(float mouse_x, float mouse_y) {
  
}

void Overlays::UpdateLevelMenu(float mouse_x, float mouse_y, float scroll)
{

}

void Overlays::UpdatePaused(float mouse_x, float mouse_y) {

}

void Overlays::DrawMenu(sf::RenderWindow& window) {
  
}

void Overlays::DrawControls(sf::RenderWindow& window) {
 
}

void Overlays::DrawTimer(sf::RenderWindow& window, int t, bool is_high_score) {
  sf::Text text;
  //the overlay is drawn more often than the 60 Hz timer ticks => play each sound once
  static int last_sound_t = -1;
  const bool new_tick = (t != last_sound_t);
  last_sound_t = t;
  if (t < 0) {
    return;
  } else if (t < 3*60) {
    //Create text for the number
    char txt[] = "0";
    txt[0] = '3' - (t / 60);
    MakeText(txt, 640, 50, 140, sf::Color::White, text);

    //Play count sound if needed
    if (t % 60 == 0 && new_tick) {
      sound_count.play();
    }
  } else if (t < 4*60) {
    MakeText("GO!", 640, 50, 140, sf::Color::White, text);

    //Play go sound if needed
    if (t == 3*60 && new_tick) {
      sound_go.play();
    }
  } else {
    //Create timer text
    const int score = t - 3 * 60;
    const sf::Color col = (is_high_score ? sf::Color::Green : sf::Color::White);
    MakeTime(score, 530, 10, 60, col, text);
  }

  if (t < 4*60) {
    //Apply zoom animation
    const float fpart = float(t % 60) / 60.0f;
    const float zoom = 0.8f + fpart*0.2f;
    const sf::Uint8 alpha = sf::Uint8(255.0f*(1.0f - fpart*fpart));
    text.setScale(sf::Vector2f(zoom, zoom));
    text.setFillColor(sf::Color(255, 255, 255, alpha));
    text.setOutlineColor(sf::Color(0, 0, 0, alpha));

    //Center text
    const sf::FloatRect text_bounds = text.getLocalBounds();
    text.setOrigin(text_bounds.width / 2, text_bounds.height / 2);
  }

  //Draw the text
  window.draw(text);
}

void Overlays::DrawLevelDesc(sf::RenderWindow& window, std::string desc) {
  sf::Text text;
  MakeText(desc.c_str(), 640, 60, 48, sf::Color::White, text);
  const sf::FloatRect text_bounds = text.getLocalBounds();
  text.setOrigin(text_bounds.width / 2, text_bounds.height / 2);
  window.draw(text);
}

void Overlays::DrawFPS(sf::RenderWindow& window, int fps) {
  sf::Text text;
  std::string fps_str = std::to_string(fps) + "fps";
  const sf::Color col = (fps < 50 ? sf::Color::Red : sf::Color::White);
  MakeText(fps_str.c_str(), 1280, 720, 24, col, text, false);
  const sf::FloatRect text_bounds = text.getLocalBounds();
  text.setOrigin(text_bounds.width, text_bounds.height);
  window.draw(text);
}

void Overlays::DrawPaused(sf::RenderWindow& window) {
  for (int i = PAUSED; i <= MOUSE; ++i) {
    window.draw(all_text[i]);
  }
}

void Overlays::DrawArrow(sf::RenderWindow& window, const sf::Vector3f& v3, const std::string& label) {
  const float x_scale = 250.0f * v3.y + 520.0f * (1.0f - v3.y);
  const float x = 640.0f + x_scale * std::cos(v3.x);
  const float y = 360.0f + 250.0f * std::sin(v3.x);
  const sf::Uint8 alpha = sf::Uint8(102.0f * std::max(0.0f, std::min(1.0f, (v3.z - 5.0f) / 30.0f)));
  if (alpha > 0) {
    arrow_spr.setScale(draw_scale * 0.1f, draw_scale * 0.1f);
    arrow_spr.setRotation(90.0f + v3.x * 180.0f / PI);
    arrow_spr.setPosition(draw_scale * x, draw_scale * y);
    arrow_spr.setColor(sf::Color(255, 255, 255, alpha));
    window.draw(arrow_spr);

    //number of the flag the arrow points to (multi flag mode)
    if (!label.empty()) {
      const float lx = 640.0f + (x_scale - 70.0f) * std::cos(v3.x);
      const float ly = 360.0f + (250.0f - 70.0f) * std::sin(v3.x);
      sf::Text text;
      MakeText(label.c_str(), lx, ly, 40, sf::Color::White, text);
      const sf::FloatRect b = text.getLocalBounds();
      text.setOrigin(b.left + b.width / 2, b.top + b.height / 2);
      const sf::Uint8 ta = sf::Uint8(std::min(255, int(alpha) * 2 + 40));
      text.setFillColor(sf::Color(255, 255, 255, ta));
      text.setOutlineColor(sf::Color(0, 0, 0, ta));
      window.draw(text);
    }
  }
}

void Overlays::DrawFlagCounter(sf::RenderWindow& window, int next, int total) {
  DrawProgressText(window, "Flag " + std::to_string(std::min(next + 1, total)) + " / " + std::to_string(total));
}

void Overlays::DrawProgressText(sf::RenderWindow& window, const std::string& str) {
  sf::Text text;
  MakeText(str.c_str(), 640, 95, 32, sf::Color(255, 230, 120), text);
  const sf::FloatRect b = text.getLocalBounds();
  text.setOrigin(b.left + b.width / 2, 0);
  window.draw(text);
}

void Overlays::DrawCredits(sf::RenderWindow& window, bool fullrun, int t) {
  std::wstring txt = LOCAL["CongratsEnd"];
  sf::Text text;
  MakeText(txt, 100, 100, 44, sf::Color::White, text);
  //text.setLineSpacing(1.3f);
  window.draw(text);

  if (fullrun) {
    sf::Text time_txt;
    MakeTime(t, 640, 226, 72, sf::Color::White, time_txt);
    const sf::FloatRect text_bounds = time_txt.getLocalBounds();
    time_txt.setOrigin(text_bounds.width / 2, text_bounds.height / 2);
    window.draw(time_txt);
  }
}

void Overlays::DrawMidPoint(sf::RenderWindow& window, bool fullrun, int t) {
  std::wstring txt = LOCAL["CongratsMid"];
  sf::Text text;
  MakeText(txt, 205, 100, 44, sf::Color::White, text);
  //text.setLineSpacing(1.3f);
  window.draw(text);

  if (fullrun) {
    sf::Text time_txt;
    MakeTime(t, 640, 226, 72, sf::Color::White, time_txt);
    const sf::FloatRect text_bounds = time_txt.getLocalBounds();
    time_txt.setOrigin(text_bounds.width / 2, text_bounds.height / 2);
    window.draw(time_txt);
  }
}

void Overlays::DrawLevels(sf::RenderWindow& window) {
  
}

void Overlays::DrawSumTime(sf::RenderWindow& window, int t) {
  sf::Text text;
  MakeTime(t, 10, 10, 32, sf::Color::White, text);
  window.draw(text);
}

void Overlays::DrawCheatsEnabled(sf::RenderWindow& window) {
  sf::Text text;
  MakeText(LOCAL["CheatsON"], 10, 680, 32, sf::Color::White, text);
  window.draw(text);
}

void Overlays::DrawCheats(sf::RenderWindow& window) {
  sf::Text text;
  std::wstring txt = LOCAL["CheatsInfo"];
  MakeText(txt, 460, 160, 32, sf::Color::White, text, true);
  window.draw(text);
}


template<class T> void Overlays::MakeText(T str, float x, float y, float size, const sf::Color& color, sf::Text& text, bool mono) {
  text.setString(str);
  text.setFont(mono ? LOCAL("mono"): LOCAL("default"));
  text.setCharacterSize(int(size * draw_scale));
  //text.setLetterSpacing(0.8f);
  text.setPosition((x - 2.0f) * draw_scale, (y - 2.0f) * draw_scale);
  text.setFillColor(color);
  text.setOutlineThickness(3.0f * draw_scale);
  text.setOutlineColor(sf::Color::Black);
}

void Overlays::MakeTime(int t, float x, float y, float size, const sf::Color& color, sf::Text& text) {
  //Create timer text
  char txt[] = "00:00:00";
  const int t_all = std::min(t, 59 * (60 * 60 + 60 + 1));
  const int t_ms = t_all % 60;
  const int t_sec = (t_all / 60) % 60;
  const int t_min = t_all / (60 * 60);
  txt[0] = '0' + t_min / 10; txt[1] = '0' + t_min % 10;
  txt[3] = '0' + t_sec / 10; txt[4] = '0' + t_sec % 10;
  txt[6] = '0' + t_ms / 10;  txt[7] = '0' + t_ms % 10;
  MakeText(txt, x, y, size, color, text, true);
}

void Overlays::UpdateHover(Texts from, Texts to, float mouse_x, float mouse_y) {
  for (int i = from; i <= to; ++i) {
    sf::FloatRect bounds = all_text[i].getGlobalBounds();
    if (bounds.contains(mouse_x, mouse_y)) {
      all_text[i].setFillColor(sf::Color(255, 64, 64));
      if (!all_hover[i]) {
        //sound_hover.play();
        all_hover[i] = true;
      }
    } else {
      all_hover[i] = false;
    }
  }
}


void Overlays::SetAntTweakBar(int Width, int Height)
{
	//TW interface
	TwInit(TW_OPENGL, NULL);
	TwWindowSize(Width, Height);
}

void Overlays::DrawAntTweakBar()
{
	//Refresh tweak bar
	if (TWBAR_ENABLED)
	{
		TwRefreshBar(stats);
		TwRefreshBar(settings);
		TwRefreshBar(fractal_editor);
		TwRefreshBar(level_editor);
		TwRefreshBar(flaunch);
		TwDraw();
	}
}

bool Overlays::TwManageEvent(sf::Event *event)
{
	bool handled = 0;

	TwMouseAction mouseAction;
	int key = 0;
	static int s_KMod = 0;
	static bool s_PreventTextHandling = false;
	static int s_WheelPos = 0;
	if(TWBAR_ENABLED)
	{
		switch (event->type)
		{
		case sf::Event::KeyPressed:
			s_PreventTextHandling = false;
			s_KMod = 0;
			if (event->key.shift)   s_KMod |= TW_KMOD_SHIFT;
			if (event->key.alt)     s_KMod |= TW_KMOD_ALT;
			if (event->key.control) s_KMod |= TW_KMOD_CTRL;
			key = 0;
			switch (event->key.code)
			{
			case sf::Keyboard::Escape:
				key = TW_KEY_ESCAPE;
				break;
			case sf::Keyboard::Return:
				key = TW_KEY_RETURN;
				break;
			case sf::Keyboard::Tab:
				key = TW_KEY_TAB;
				break;
			case sf::Keyboard::BackSpace:
				key = TW_KEY_BACKSPACE;
				break;
			case sf::Keyboard::PageUp:
				key = TW_KEY_PAGE_UP;
				break;
			case sf::Keyboard::PageDown:
				key = TW_KEY_PAGE_DOWN;
				break;
			case sf::Keyboard::Up:
				key = TW_KEY_UP;
				break;
			case sf::Keyboard::Down:
				key = TW_KEY_DOWN;
				break;
			case sf::Keyboard::Left:
				key = TW_KEY_LEFT;
				break;
			case sf::Keyboard::Right:
				key = TW_KEY_RIGHT;
				break;
			case sf::Keyboard::End:
				key = TW_KEY_END;
				break;
			case sf::Keyboard::Home:
				key = TW_KEY_HOME;
				break;
			case sf::Keyboard::Insert:
				key = TW_KEY_INSERT;
				break;
			case sf::Keyboard::Delete:
				key = TW_KEY_DELETE;
				break;
			case sf::Keyboard::Space:
				key = TW_KEY_SPACE;
				break;
			default:
				if (event->key.code >= sf::Keyboard::F1 && event->key.code <= sf::Keyboard::F15)
					key = TW_KEY_F1 + event->key.code - sf::Keyboard::F1;
				else if (s_KMod & TW_KMOD_ALT)
				{
					if (event->key.code >= sf::Keyboard::A && event->key.code <= sf::Keyboard::Z)
					{
						if (s_KMod & TW_KMOD_SHIFT)
							key = 'A' + event->key.code - sf::Keyboard::A;
						else
							key = 'a' + event->key.code - sf::Keyboard::A;
					}
				}
			}
			if (key != 0)
			{
				handled = TwKeyPressed(key, s_KMod);
				s_PreventTextHandling = true;
			}
			break;
		case sf::Event::KeyReleased:
			s_PreventTextHandling = false;
			s_KMod = 0;
			break;
		case sf::Event::TextEntered:
			if (!s_PreventTextHandling && event->text.unicode != 0 && (event->text.unicode & 0xFF00) == 0)
			{
				if ((event->text.unicode & 0xFF) < 32) // CTRL+letter
					handled = TwKeyPressed((event->text.unicode & 0xFF) + 'a' - 1, TW_KMOD_CTRL | s_KMod);
				else
					handled = TwKeyPressed(event->text.unicode & 0xFF, 0);
			}
			s_PreventTextHandling = false;
			break;
		case sf::Event::MouseMoved:
			handled = TwMouseMotion(event->mouseMove.x, event->mouseMove.y);
			break;
		case sf::Event::MouseButtonPressed:
		case sf::Event::MouseButtonReleased:
			mouseAction = (event->type == sf::Event::MouseButtonPressed) ? TW_MOUSE_PRESSED : TW_MOUSE_RELEASED;
			switch (event->mouseButton.button)
			{
			case sf::Mouse::Left:
				handled = TwMouseButton(mouseAction, TW_MOUSE_LEFT);
				break;
			case sf::Mouse::Middle:
				handled = TwMouseButton(mouseAction, TW_MOUSE_MIDDLE);
				break;
			case sf::Mouse::Right:
				handled = TwMouseButton(mouseAction, TW_MOUSE_RIGHT);
				break;
			default:
				break;
			}
			break;
		case sf::Event::MouseWheelMoved:
			s_WheelPos += event->mouseWheel.delta;
			handled = TwMouseWheel(s_WheelPos);
			break;
		default:
			break;
		}
	}

	if (sf::Event::Resized == event->type)
	{
		TwWindowSize(event->size.width, event->size.height);
	}

	return handled;
}

void Overlays::SetTWBARResolution(int Width, int Height)
{
	TwWindowSize(Width, Height);
}

static std::string FormatTime(int frames) {
  const int total_cs = frames * 100 / 60;
  const int m = total_cs / 6000, sec = (total_cs / 100) % 60, cs = total_cs % 100;
  char buf[32];
  snprintf(buf, sizeof(buf), "%d:%02d.%02d", m, sec, cs);
  return buf;
}

//split time when a checkpoint / flag is reached (total time, it keeps running)
void Overlays::DrawSplit(sf::RenderWindow& window, const std::string& label, int t, int show) {
  sf::Text text;
  const std::string str = label + "   " + FormatTime(t);
  MakeText(str.c_str(), 640, 140, 36, sf::Color(120, 255, 140), text);
  const sf::FloatRect b = text.getLocalBounds();
  text.setOrigin(b.left + b.width / 2, 0);
  const sf::Uint8 a = sf::Uint8(std::min(255, show * 8));
  text.setFillColor(sf::Color(120, 255, 140, a));
  text.setOutlineColor(sf::Color(0, 0, 0, a));
  window.draw(text);
}

//remaining time of the level time limit
void Overlays::DrawTimeLeft(sf::RenderWindow& window, int frames_left) {
  sf::Text text;
  const sf::Color col = (frames_left < 10 * 60) ? sf::Color(255, 70, 50) : sf::Color(255, 200, 80);
  MakeText(("Time left " + FormatTime(frames_left)).c_str(), 1260, 10, 36, col, text);
  const sf::FloatRect b = text.getLocalBounds();
  text.setOrigin(b.left + b.width, 0);
  window.draw(text);
}

void Overlays::DrawEditorMarker(sf::RenderWindow& window, float x, float y, const std::string& label, const sf::Color& col, bool selected) {
  const float s = std::max(draw_scale, 0.5f);
  //label box above the object, a line down to the exact point
  sf::Text text;
  text.setString(label);
  text.setFont(LOCAL("default"));
  text.setCharacterSize(int(20 * s));
  text.setFillColor(selected ? sf::Color(255, 60, 60) : col);
  text.setOutlineThickness(2.0f * s);
  text.setOutlineColor(sf::Color::Black);
  const sf::FloatRect b = text.getLocalBounds();
  const float lift = 28.0f * s;
  text.setOrigin(b.left + b.width / 2, b.top + b.height);
  text.setPosition(x, y - lift);

  sf::RectangleShape bg(sf::Vector2f(b.width + 12 * s, b.height + 10 * s));
  bg.setOrigin(bg.getSize().x / 2, bg.getSize().y - 5 * s);
  bg.setPosition(x, y - lift);
  bg.setFillColor(sf::Color(0, 0, 0, 150));
  bg.setOutlineThickness(1.5f * s);
  bg.setOutlineColor(selected ? sf::Color(255, 60, 60) : col);

  sf::Vertex line[] = { sf::Vertex(sf::Vector2f(x, y - lift + 5 * s), col), sf::Vertex(sf::Vector2f(x, y), col) };
  sf::CircleShape dot(4.0f * s);
  dot.setOrigin(4.0f * s, 4.0f * s);
  dot.setPosition(x, y);
  dot.setFillColor(selected ? sf::Color(255, 60, 60) : col);
  dot.setOutlineThickness(1.0f * s);
  dot.setOutlineColor(sf::Color::Black);

  window.draw(line, 2, sf::Lines);
  window.draw(bg);
  window.draw(text);
  window.draw(dot);
}
