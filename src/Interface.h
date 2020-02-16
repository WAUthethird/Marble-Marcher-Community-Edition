#pragma once
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <Localization.h>
#include <map>
#include <stack>
#include <algorithm>
#include <iterator>
#include <functional>
#include <stack>

#ifdef _WIN32
#define ERROR_MSG(x) MessageBox(nullptr, TEXT(x), TEXT("ERROR"), MB_OK);
#else
#define ERROR_MSG(x) std::cerr << x << std::endl;
#endif

//default interface parameters
extern sf::Color default_main_color;
extern sf::Color default_hover_main_color;
extern sf::Color default_active_main_color;
extern float default_margin;
extern sf::Vector2f default_size;
extern sf::View default_view;

extern float animation_sharpness;
extern float action_dt;

static const std::string close_png = "images/clear.png";
static const std::string delete_png = "images/delete.png";
static const std::string edit_png = "images/edit.png";
static const std::string priority_png = "images/priority.png";
static const std::string done_png = "images/done.png";
extern int focused;

template < typename T > std::string num2str(const T& n)
{
	std::ostringstream stm;
	if (n < 10) stm << "0";
	stm << n;
	return stm.str();
}

struct ColorFloat
{
	float r, g, b, a;
	ColorFloat(float red = 0.f, float green = 0.f, float blue = 0.f, float alpha = 255.f);

	void operator=(sf::Color a);
};

ColorFloat operator+(ColorFloat a, ColorFloat b);

ColorFloat operator-(ColorFloat a, ColorFloat b);

ColorFloat operator*(ColorFloat a, float b);

sf::Color ToColor(ColorFloat a);
ColorFloat ToColorF(sf::Color a);

struct InputState
{
	bool isKeyPressed;
	bool keys[sf::Keyboard::KeyCount] = { false };
	bool key_press[sf::Keyboard::KeyCount] = { false };
	bool mouse[3] = { false };
	bool mouse_press[3] = { false };
	float wheel = 0.f;
	sf::Vector2f mouse_pos = sf::Vector2f(0,0);
	sf::Vector2f mouse_prev = sf::Vector2f(0, 0);
	sf::Vector2f mouse_speed = sf::Vector2f(0, 0);
	sf::Vector2f window_size = sf::Vector2f(0, 0);
	float time = 0, dt = 0;
	bool axis_moved[sf::Joystick::AxisCount] = { false };
	float axis_value[sf::Joystick::AxisCount] = { 0.f };
	bool  buttons[sf::Joystick::ButtonCount] = { false };
	bool  button_pressed[sf::Joystick::ButtonCount] = { false };

	InputState();
	InputState(bool keys[sf::Keyboard::KeyCount], bool mouse[3], sf::Vector2f mouse_pos, sf::Vector2f mouse_speed);
};

typedef std::function<void(sf::RenderWindow * window, InputState & state)> call_func;

//the object parameters
struct State
{
	sf::Vector2f position = sf::Vector2f(0,0);
	sf::Vector2f size = sf::Vector2f(0.1f, 0.1f);
	float border_thickness = 0.f;
	float margin = default_margin;
	float font_size = 1.f;
	float scroll = 0.f;
	float inside_size = 0.f;
	ColorFloat color_main = ToColorF(sf::Color::Black);
	ColorFloat color_second = ToColorF(sf::Color::Transparent);
	ColorFloat color_border = ColorFloat(200,0,0);
};

//interpolate between states for animation effects
State interpolate(State a, State b, float t);

void UpdateAllObjects(sf::RenderWindow * window, InputState& state);


//the object base class
class Object
{
public:
	enum States
	{
		DEFAULT, ONHOVER, ACTIVE
	};

	enum Allign
	{
		LEFT, CENTER, RIGHT
	};

	void ApplyScroll(float x);
	void SetPosition(float x, float y);
	void SetSize(float x, float y);
	void SetHeigth(float x);
	void SetWidth(float x);
	void SetBackgroundColor(sf::Color color);
	void SetBorderColor(sf::Color color);
	void SetBorderWidth(float S);
	void SetMargin(float x);
	void SetInsideSize(float x);
	void SetScroll(float x);
	void Move(sf::Vector2f dx);

	void SetDefaultFunction(call_func fun);
	void SetCallbackFunction(call_func fun, bool limit_repeat = true);
	void SetHoverFunction(call_func fun);
	
	void SetMainDefaultFunction(call_func fun);
	void SetMainCallbackFunction(call_func fun, bool limit_repeat = true);
	void SetMainHoverFunction(call_func fun);

	void ClearDefaultFunctions();
	void ClearCallbackFunctions();
	void ClearHoverFunctions();

	bool RunCallback(sf::RenderWindow * window, InputState& state);
	void clone_states();

	virtual void Draw(sf::RenderWindow * window, InputState& state);
	virtual void AddObject(Object* a, Allign b);
	void Update(sf::RenderWindow * window, InputState& state);
	void UpdateAction(sf::RenderWindow * window, InputState& state);
	
	Object();

	Object(Object& A);
	Object(Object&& A);
	Object(Object* A);

	virtual void operator=(Object& A);
	virtual void operator=(Object&& A);

	void copy(Object& A);
	void copy(Object&& A);

	virtual Object* GetCopy();

	State curstate;
	State activestate;
	State hoverstate;
	State defaultstate;
	States curmode;

	Allign obj_allign;

	sf::View used_view;
	
	//multiple callbacks
	std::vector<std::function<void(sf::RenderWindow * window, InputState & state)>> callback, hoverfn, defaultfn;

	//objects inside this object
	std::vector<std::unique_ptr<Object>> objects;
	std::stack<int> z_buffer;

	//operation time limiter
	float action_time;
	bool static_object;
	bool limiter;

	sf::Vector2f worldPos;
	sf::FloatRect obj;

	int id;
};

void UpdateAspectRatio(float width, float heigth);
int AddGlobalObject(Object & a);
Object& get_glob_obj(int id);
bool NoObjects();
int NumberOfObjects();
void RemoveGlobalObject(int id);
void RemoveAllObjects();
void Add2DeleteQueue(int id);

//a box to add stuff in
class Box: public Object
{
public:
	void SetBackground(const sf::Texture & texture);
	void Draw(sf::RenderWindow *window, InputState& state);

	Box(float x, float y,  float dx, float dy,  sf::Color color_main = default_main_color);
	Box(float dx, float dy, sf::Color color_main = default_main_color);
	Box();

	Box(Box& A);
	Box(Box&& A);

	void SetAutoSize(bool b);

	void operator=(Box& A);
	void operator=(Box&& A);

	virtual Object* GetCopy();

	~Box();

protected:
	bool auto_size;

private:
	sf::Texture image;
	sf::RectangleShape rect;
	sf::View boxView;
};



class MenuBox : public Box
{
public:
	int cursor_id;

	sf::Vector2f dmouse;

	virtual void AddObject(Object * something, Allign a);

	void Cursor(int d);

	void ScrollBy(float dx);
	void ScrollTo(float scroll);

	MenuBox(float dx, float dy, bool auto_y = false, float x = 0, float y = 0, sf::Color color_main = sf::Color(0, 0, 0, 0));

	MenuBox(MenuBox& A);
	MenuBox(MenuBox&& A);

	void CreateCallbacks();

	void operator=(MenuBox& A);
	void operator=(MenuBox&& A);

	virtual Object* GetCopy();
};


class Window: public Box
{
public:
	sf::Vector2f dmouse;

	void Add(Object * something, Allign a = LEFT);

	template<class T>
	Window(float x, float y, float dx, float dy, sf::Color color_main = default_main_color, T title = LOCAL["Window"], sf::Font & font = LOCAL("default"));

	Window(Window& A);
	Window(Window&& A);

	void CreateCallbacks();

	void operator=(Window& A);
	void operator=(Window&& A);

	virtual Object* GetCopy();
};


class Text : public Object
{
public:
	std::unique_ptr<sf::Text> text;

	void Draw(sf::RenderWindow *window, InputState& state);

	template<class T>
	Text(T str, sf::Font &f, float size, sf::Color col = sf::Color::White);
	Text(sf::Text t);

	Text(Text& A);
	Text(Text&& A);

	template<class T>
	void SetString(T str);

	void operator=(Text& A);
	void operator=(Text&& A);

	virtual Object* GetCopy();
};


//a button
class Button: public Box
{
public:
	template<class T>
	Button(T text, float w, float h, std::function<void(sf::RenderWindow * window, InputState & state)> fun, sf::Color color_hover = default_hover_main_color, sf::Color color_main = default_main_color);

	Button(Button& A);
	Button(Button&& A);

	void operator=(Button& A);
	void operator=(Button&& A);

	virtual Object* GetCopy();

	~Button();
};

class Image : public Box
{
public:
	Image(sf::Texture image, float w, float h, sf::Color color_hover);
	Image(std::string image, float w = 0, float h = 0, sf::Color color_hover = sf::Color::White);

	Image(Image& A);
	Image(Image&& A);

	void operator=(Image& A);
	void operator=(Image&& A);

	virtual Object* GetCopy();

	~Image();
};


std::string key_name(sf::Keyboard::Key& key);

//basic object for control mapping
class KeyMapper : public Box
{
public:
	enum MapperType {
		KEYBOARD, JOYSTICK_AXIS, JOYSTICK_KEYS
	};

	template<class T>
	KeyMapper(T label, T act_label, int* key, float w, float h, MapperType type, sf::Color color_active = sf::Color::Blue, sf::Color color_hover = default_hover_main_color, sf::Color color_main = default_main_color);

	KeyMapper(KeyMapper& A);
	KeyMapper(KeyMapper&& A);

	void operator=(KeyMapper& A);
	void operator=(KeyMapper&& A);

	void CreateCallbacks();
	void SetKeyString();

	virtual Object* GetCopy();

	MapperType this_type;
	int *key_ptr;
	bool waiting;
	std::wstring wait_text;
};

template<class T>
inline KeyMapper::KeyMapper(T label, T act_label, int * key, float w, float h, MapperType type, sf::Color color_active, sf::Color color_hover, sf::Color color_main): waiting(false), this_type(type), wait_text(act_label)
{
	SetSize(w, h);
	SetBackgroundColor(color_main);
	Box LeftBox(0, 0, float(w * 0.5f), h-defaultstate.margin*2.f, sf::Color(0, 100, 200, 0)),
		RightBox(0, 0, float(w * 0.33f), h - defaultstate.margin * 2.f, sf::Color(0, 100, 200, 128));
	LeftBox.AddObject(new Text(label, LOCAL("default"), h*0.7f, sf::Color::White), Allign::LEFT);
	RightBox.AddObject(new Text(act_label, LOCAL("default"), h*0.7f, sf::Color::White), Allign::CENTER);
	RightBox.activestate.color_main = color_active;
	hoverstate.color_main = color_hover;
	this->AddObject(&LeftBox,Allign::LEFT);
	this->AddObject(&RightBox, Allign::RIGHT);
	key_ptr = key;
	SetKeyString();
}

template<class T>
inline Window::Window(float x, float y, float dx, float dy, sf::Color color_main, T title, sf::Font & font)
{
	defaultstate.position.x = x;
	defaultstate.position.y = y;
	defaultstate.size.x = dx;
	defaultstate.size.y = dy;

	this->SetAutoSize(true);
	defaultstate.color_main = ToColorF(color_main);
	clone_states();
	SetMargin(0);

	Box Bar(0, 0, dx, 30, sf::Color(0, 100, 200, 128)),
		CloseBx(0, 0, 30, 30, sf::Color(255, 255, 255, 255));
	Text Title(title, font, 25, sf::Color::White);
	Bar.SetMargin(0);
	CloseBx.hoverstate.color_main = sf::Color(255, 0, 0, 255);

	sf::Image close; close.loadFromFile(close_png);
	sf::Texture closetxt; closetxt.loadFromImage(close);
	closetxt.setSmooth(true);
	CloseBx.SetBackground(closetxt);

	Bar.AddObject(&Title, Box::LEFT);
	Bar.AddObject(&CloseBx, Box::RIGHT);

	MenuBox Inside(dx, dy - 30, true);


	this->AddObject(&Bar, Box::CENTER);
	this->AddObject(&Inside, Box::LEFT);

	CreateCallbacks();
}

template<class T>
inline Button::Button(T text, float w, float h, std::function<void(sf::RenderWindow*window, InputState&state)> fun, sf::Color color_hover, sf::Color color_main)
{
	SetSize(w, h);
	SetBackgroundColor(color_main);
	Text button_text(text, LOCAL("default"), h*0.8f, sf::Color::White);
	button_text.SetBorderColor(sf::Color::Black);
	hoverstate.color_main = color_hover;
	SetCallbackFunction(fun, true);
	this->AddObject(&button_text,Object::Allign::CENTER);
}



template<class T>
inline Text::Text(T str, sf::Font & f, float size, sf::Color col)
{
	text.reset(new sf::Text(str, f, size));
	defaultstate.font_size = size;
	defaultstate.color_main = col;
	SetBorderColor(sf::Color::Black);
	SetBorderWidth(2);
	clone_states();
}

template<class T>
inline void Text::SetString(T str)
{
	text.get()->setString(str);
}
