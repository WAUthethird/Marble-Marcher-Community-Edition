#include <Interface.h>

std::map<int, std::unique_ptr<Object>> global_objects;
std::vector<int> z_value; //rendering order
std::map<int, int> z_index;
std::vector<int> del; // deletion stack
int all_obj_id = 0;
std::map<int, bool> active;
int focused = 0;
int cursor = 0;
int global_focus = 0;

sf::Color default_main_color = sf::Color(64, 64, 64, 128);
sf::Color default_hover_main_color = sf::Color(200, 128, 128, 128);
sf::Color default_active_main_color = sf::Color(255, 128, 128, 255);
float default_margin =2;
sf::Vector2f default_size = sf::Vector2f(1920, 1080);
sf::View default_view = sf::View(sf::FloatRect(0, 0, default_size.x, default_size.y));

float animation_sharpness = 5.f;
float action_dt = 0.3;


sf::FloatRect overlap(sf::FloatRect a, sf::FloatRect b)
{
	sf::FloatRect c;
	c.top = std::max(a.top, b.top);
	c.left = std::max(a.left, b.left);
	c.height = std::max(std::min(a.top + a.height, b.top + b.height) - c.top, 0.f);
	c.width = std::max(std::min(a.left + a.width, b.left + b.width) - c.left, 0.f);
	return c;
}

ColorFloat operator+(ColorFloat a, ColorFloat b)
{
	ColorFloat c;
	c.r = a.r + b.r;
	c.g = a.g + b.g;
	c.b = a.b + b.b;
	c.a = a.a + b.a;
	return c;
}

ColorFloat operator-(ColorFloat a, ColorFloat b)
{
	ColorFloat c;
	c.r = a.r - b.r;
	c.g = a.g - b.g;
	c.b = a.b - b.b;
	c.a = a.a - b.a;
	return c;
}

ColorFloat operator*(ColorFloat a, float b)
{
	ColorFloat c;
	c.r = a.r * b;
	c.g = a.g * b;
	c.b = a.b * b;
	c.a = a.a * b;
	return c;
}

sf::Color ToColor(ColorFloat a)
{
	return sf::Color(a.r, a.g, a.b, a.a);
}

ColorFloat ToColorF(sf::Color a)
{
	return ColorFloat(a.r, a.g, a.b, a.a);
}

Object& get_glob_obj(int id)
{
	return *global_objects[id].get();
}

void UpdateAspectRatio(float width, float heigth)
{
	sf::Vector2f size = sf::Vector2f(default_size.x * std::max(1.f, (width / heigth) * (default_size.y / default_size.x)), default_size.x * std::max(default_size.y / default_size.x, heigth / width));
	default_view.reset(sf::FloatRect(sf::Vector2f(0, 0), size));
}

int AddGlobalObject(Object & a)
{
	Object* copy = a.GetCopy();
	global_objects[copy->id] = std::unique_ptr<Object>(copy);//add a copy to the global list
	global_objects[copy->id].get()->id = copy->id;
	z_value.push_back(copy->id);
	global_focus = copy->id;
	focused = copy->id;
	return copy->id;  
}

bool global_exists(int id)
{
	for (auto &obj : global_objects)
	{
		if (obj.first == id)
		{
			return true;
		}
	}
	return false;
}

int z_val(int id)
{
	std::vector<int>::iterator it = std::find(z_value.begin(), z_value.end(), id);
	return std::distance(z_value.begin(), it);
}

bool NoObjects()
{
	if (global_objects.size() != 0)
	{
		return false;
	}
	return true;
}

int NumberOfObjects()
{
	return global_objects.size();
}

void RemoveGlobalObject(int id)
{
	if (global_objects.count(id) != 0)
	{
		global_objects.erase(id);	
		int z_id = z_val(id);
		z_value.erase(z_value.begin() + z_id);
	}
}

void RemoveAllObjects1()
{
	global_objects.clear();
	z_value.clear();
}


void RemoveAllObjects()
{
	for (auto &obj:global_objects)
	{
		del.push_back(obj.first);
	}
}

void Add2DeleteQueue(int id)
{
	if (global_objects.count(id) != 0)
	{
		del.push_back(id);
	}
}

std::string key_name(sf::Keyboard::Key & key)
{
	//yeah, I dont like this code either
#define ITEM(x) case sf::Keyboard::x : return #x;
	switch (key)
	{
		ITEM(A); ITEM(B); ITEM(C);
		ITEM(D); ITEM(E); ITEM(F); ITEM(G);
		ITEM(H); ITEM(I); ITEM(J); ITEM(K);
		ITEM(L); ITEM(M); ITEM(N); ITEM(O);
		ITEM(P); ITEM(Q); ITEM(R); ITEM(S);
		ITEM(T); ITEM(U); ITEM(V); ITEM(W);
		ITEM(X); ITEM(Y); ITEM(Z); ITEM(Num0);
		ITEM(Num1); ITEM(Num2); ITEM(Num3); ITEM(Num4);
		ITEM(Num5); ITEM(Num6); ITEM(Num7); ITEM(Num8);
		ITEM(Num9); ITEM(Escape); ITEM(LControl); ITEM(LShift);
		ITEM(LAlt); ITEM(LSystem); ITEM(RControl); ITEM(RShift);
		ITEM(RAlt); ITEM(RSystem); ITEM(Menu); ITEM(LBracket);
		ITEM(RBracket); /*ITEM(Semicolon);*/ ITEM(Comma); ITEM(Period);
		ITEM(Quote); ITEM(Slash); /*ITEM(Backslash);*/ ITEM(Tilde);
		ITEM(Equal);/* ITEM(Hyphen);*/ ITEM(Space); /*ITEM(Enter);*/
		/*ITEM(Backspace); */ITEM(Tab); ITEM(PageUp); ITEM(PageDown);
		ITEM(End); ITEM(Home); ITEM(Insert); ITEM(Delete);
		ITEM(Add); ITEM(Subtract); ITEM(Multiply); ITEM(Divide);
		ITEM(Left); ITEM(Right); ITEM(Up); ITEM(Down);
		ITEM(Numpad0); ITEM(Numpad1); ITEM(Numpad2); ITEM(Numpad3);
		ITEM(Numpad4); ITEM(Numpad5); ITEM(Numpad6); ITEM(Numpad7);
		ITEM(Numpad8); ITEM(Numpad9); ITEM(F1); ITEM(F2);
		ITEM(F3); ITEM(F4); ITEM(F5); ITEM(F6);
		ITEM(F7); ITEM(F8); ITEM(F9); ITEM(F10);
		ITEM(F11); ITEM(F12); ITEM(F13); ITEM(F14);
		ITEM(F15); ITEM(Pause);
	default:
		return "UNKNOWN";
	}
}

void UpdateAllObjects(sf::RenderWindow * window, InputState& state)
{
	for (auto &id : del)
	{
		RemoveGlobalObject(id);
	}

	del.clear();
	
	//render stuff in the following order
	for (auto &z : z_value)
	{
		if (global_objects.count(z) != 0) 
		{
			global_objects[z].get()->used_view = default_view;
			global_objects[z].get()->Update(window, state);
		}
	}
	
	if (global_objects.count(global_focus) != 0)
	{
		//put the focused global object to the top
		int top_id = z_value[z_value.size() - 1];

		//if not the top object and not a static object
		if (top_id != global_focus)
		{
			if (global_objects.count(top_id) != 0)
			{
				global_objects[top_id]->UpdateAction(window, state);
				if (!global_objects[global_focus].get()->static_object)
				{
					int global_z = z_val(global_focus);
					int top_z = z_value.size() - 1;
					std::swap(z_value[top_z], z_value[global_z]);
				}
			}
		}
		
		//update callbacks in the focused object
		global_objects[global_focus]->UpdateAction(window, state);
	}
}


State interpolate(State a, State b, float t)
{
	State C;
	C.position.x = a.position.x*(1.f - t) + b.position.x*t;
	C.position.y = a.position.y*(1.f - t) + b.position.y*t;
	C.size.x = a.size.x*(1.f - t) + b.size.x*t;
	C.size.y = a.size.y*(1.f - t) + b.size.y*t;
	C.border_thickness = a.border_thickness*(1.f - t) + b.border_thickness*t;
	C.margin = a.margin*(1.f - t) + b.margin*t;
	C.scroll = a.scroll*(1.f - t) + b.scroll*t;
	C.font_size = a.font_size*(1.f - t) + b.font_size*t;
	C.color_main = a.color_main*(1.f - t) + b.color_main*t;
	C.color_second = a.color_second*(1.f - t) + b.color_second*t;
	C.color_border = a.color_border*(1.f - t) + b.color_border*t;
	return C;
}

void Object::SetPosition(float x, float y)
{
	defaultstate.position = sf::Vector2f(x, y);
	curstate.position = sf::Vector2f(x, y);
	activestate.position = sf::Vector2f(x, y);
	hoverstate.position = sf::Vector2f(x, y);
}

void Object::SetSize(float x, float y)
{
	defaultstate.size = sf::Vector2f(x, y);
	activestate.size = sf::Vector2f(x, y);
	hoverstate.size = sf::Vector2f(x, y);
}

void Object::SetHeigth(float x)
{
	defaultstate.size.y = x;
	activestate.size.y = x;
	hoverstate.size.y = x;
}

void Object::SetWidth(float x)
{
	defaultstate.size.x = x;
	activestate.size.x = x;
	hoverstate.size.x = x;
}

void Object::SetBackgroundColor(sf::Color color)
{
	defaultstate.color_main = color;
	activestate.color_main = color;
	hoverstate.color_main = color;
}

void Object::SetBorderColor(sf::Color color)
{
	defaultstate.color_border = color;
	activestate.color_border = color;
	hoverstate.color_border = color;
}

void Object::SetBorderWidth(float S)
{
	defaultstate.border_thickness = S;
	activestate.border_thickness = S;
	hoverstate.border_thickness = S;
}

void Object::SetMargin(float x)
{
	defaultstate.margin = x;
	activestate.margin = x;
	hoverstate.margin = x;
}

void Object::SetInsideSize(float x)
{
	curstate.inside_size = x;
	defaultstate.inside_size = x;
	activestate.inside_size = x;
	hoverstate.inside_size = x;
}

void Object::SetScroll(float x)
{
	defaultstate.scroll = x;
	activestate.scroll = x;
	hoverstate.scroll = x;
	curstate.scroll = x;
}

void Object::ApplyScroll(float x)
{
	defaultstate.scroll = x;
	activestate.scroll = x;
	hoverstate.scroll = x;
}

void Object::Move(sf::Vector2f dx)
{
	if (!isnan(dx.x) && !isnan(dx.y) && (dx.x != 0 || dx.y != 0))
	{
		defaultstate.position += dx;
		curstate.position += dx;
		activestate.position += dx;
		hoverstate.position += dx;
	}
}

void Object::SetDefaultFunction(call_func fun)
{
	defaultfn.push_back(fun);
}

void Object::SetCallbackFunction(call_func fun, bool limit_repeat)
{
	callback.push_back(fun);
	limiter = limit_repeat;
}

void Object::SetHoverFunction(call_func fun)
{
	hoverfn.push_back(fun);
}

void Object::SetMainDefaultFunction(call_func fun)
{
	if (defaultfn.size() == 0)
		defaultfn.push_back(fun);
	else
		defaultfn[0] = fun;
}

void Object::SetMainCallbackFunction(call_func fun, bool limit_repeat)
{
	if (callback.size() == 0)
		callback.push_back(fun);
	else
		callback[0] = fun;
	limiter = limit_repeat;
}


void Object::SetMainHoverFunction(call_func fun)
{
	if (hoverfn.size() == 0)
		hoverfn.push_back(fun);
	else
		hoverfn[0] = fun;
}

void Object::ClearDefaultFunctions()
{
	defaultfn.clear();
}

void Object::ClearCallbackFunctions()
{
	callback.clear();
}

void Object::ClearHoverFunctions()
{
	hoverfn.clear();
}

bool Object::RunCallback(sf::RenderWindow * window, InputState & state)
{
	if (callback.size() != 0)
	{
		//run through all of the callbacks
		for (auto &this_callback : callback)
		{
			this_callback(window, state); //run callback with state info
		}
		return true;
	}
	else
	{
		//try to run a callback inside the object
		for (auto &obj : objects)
		{
			if (obj.get()->RunCallback(window, state))
			{
				return true;
			}
		}

		//mission failed. we'll get 'em next time
		return false;
	}
}

void Object::clone_states()
{
	curstate = defaultstate;
	activestate = defaultstate;
	hoverstate = defaultstate;
}

void Object::Update(sf::RenderWindow * window, InputState& state)
{
	worldPos = window->mapPixelToCoords(sf::Vector2i(state.mouse_pos.x, state.mouse_pos.y));
	obj= sf::FloatRect(curstate.position.x, curstate.position.y, curstate.size.x, curstate.size.y);

	window->setView(used_view);
	state.mouse_speed = window->mapPixelToCoords(sf::Vector2i(state.mouse_pos.x, state.mouse_pos.y)) -
						window->mapPixelToCoords(sf::Vector2i(state.mouse_prev.x, state.mouse_prev.y));

	if ( ((state.mouse[0] || state.mouse[2]) && !limiter) || ((state.mouse_press[0] || state.mouse_press[2]) && limiter) ) //if clicked
	{
		if (obj.contains(worldPos)) // if inside object
		{
			active[id] = true; //set as active
			cursor = id;
			if (defaultfn.size() != 0)
				focused = id; // save this object as the last focused if it has a default callback

			if (global_exists(id))
			{
				global_focus = id;
			}
		}
	}
	else
	{
		active[id] = false; //deactivate
	}
	

	float t = 1.f - exp(-animation_sharpness * state.dt);

	//update animation
	switch (curmode)
	{
	case DEFAULT:
		curstate = interpolate(curstate, defaultstate, t);
		break;
	case ACTIVE:
		curstate = interpolate(curstate, activestate, t);
		break;
	case ONHOVER:
		curstate = interpolate(curstate, hoverstate, t);
		break;
	}

	Draw(window, state);
}

void Object::UpdateAction(sf::RenderWindow * window, InputState & state)
{
	state.mouse_speed = window->mapPixelToCoords(sf::Vector2i(state.mouse_pos.x, state.mouse_pos.y)) -
						window->mapPixelToCoords(sf::Vector2i(state.mouse_prev.x, state.mouse_prev.y));

	curmode = Object::DEFAULT;
	//if mouse is inside the object 
	if (obj.contains(worldPos))
	{
		if(!(state.mouse[0] || state.mouse[2])) // if hover
		{
			for (auto &this_callback : hoverfn)
			{
				this_callback(window, state); //run callback with state info
			}
			curmode = ONHOVER;
		}
	}

	if (cursor == id)
	{
		curmode = ONHOVER;
	}

	if (active[id])
	{
		for (auto &this_callback : callback)
		{
			this_callback(window, state); //run callback with state info
		}
		curmode = ACTIVE;
	}

	if (action_time <= 0.f) //limit the repetability
	{
		if (focused == id) //if this object is the one that is currently in focus
		{
			for (auto &this_callback : defaultfn)
			{
				this_callback(window, state); //run callback with state info
			}
		}
	}
	else
	{
		action_time -= state.dt;
	}

	//update callbacks for all functions inside
	for (auto &obj : objects)
	{
		obj.get()->UpdateAction(window, state);
	}
}


Object::Object() : callback(NULL), hoverfn(NULL), defaultfn(NULL), curmode(DEFAULT), action_time(0.2), obj_allign(LEFT), static_object(false), limiter(false)
{
	curstate.color_main = default_main_color;
	activestate.color_main = default_active_main_color;
	hoverstate.color_main = default_hover_main_color;
	defaultstate.color_main = default_main_color;

	used_view = default_view;

	id = all_obj_id;
	active[id] = false;
	all_obj_id++;
}

Object::Object(Object & A)
{
	*this = A;
}

Object::Object(Object && A)
{
	*this = A;
}

Object::Object(Object * A)
{
	*this = *A;
}

void Object::operator=(Object & A)
{
	copy(A);
}

void Object::operator=(Object && A)
{
	copy(A);
}

void Object::copy(Object & A)
{
	curstate = A.curstate;
	curstate.margin = A.defaultstate.margin;
	activestate = A.activestate;
	hoverstate = A.hoverstate;
	defaultstate = A.defaultstate;
	curmode = A.curmode;
	static_object = A.static_object;
	limiter = A.limiter;
	used_view = A.used_view;
	obj_allign = A.obj_allign;

	for (auto &a : A.callback) callback.push_back(a);
	for (auto &a : A.hoverfn) hoverfn.push_back(a);
	for (auto &a : A.defaultfn) defaultfn.push_back(a);

	id = all_obj_id;
	all_obj_id++;
	active[id] = false;

	action_time = A.action_time;

	for (auto &element : A.objects)
		Object::AddObject(element.get(), element.get()->obj_allign);
}

void Object::copy(Object && A)
{
	std::swap(curstate, A.curstate);
	std::swap(activestate, A.activestate);
	std::swap(hoverstate, A.hoverstate);
	std::swap(defaultstate, A.defaultstate);
	std::swap(curmode, A.curmode);
	std::swap(limiter, A.limiter);
	std::swap(static_object, A.static_object);
	std::swap(used_view, A.used_view);
	std::swap(obj_allign, A.obj_allign);
	std::swap(id, A.id);
	std::swap(action_time, A.action_time);
	std::swap(objects, A.objects);
}

Object * Object::GetCopy()
{
	return new Object(*this);
}

void Object::Draw(sf::RenderWindow * window, InputState & state)
{
	//nothing to draw
}

void Object::AddObject(Object * something, Allign a)
{
	something->obj_allign = a;
	objects.push_back(std::unique_ptr<Object>(something->GetCopy()));
	this->SetInsideSize(defaultstate.inside_size + something->defaultstate.size.y + something->defaultstate.margin);
}

void Box::SetBackground(const sf::Texture & texture)
{
	image = texture;
	rect.setTexture(&image);
}

void Box::Draw(sf::RenderWindow * window, InputState& state)
{
	if (auto_size)
	{
		SetSize(this->defaultstate.size.x, this->defaultstate.inside_size + this->defaultstate.margin);
	}

	//update the box itself
	if (   !(ToColor(defaultstate.color_main) == sf::Color::Transparent && curmode == DEFAULT)
		&& !(ToColor(hoverstate.color_main) == sf::Color::Transparent && curmode == ONHOVER)
		&& !(ToColor(activestate.color_main) == sf::Color::Transparent && curmode == ACTIVE)  )
	{
		rect.setPosition(curstate.position);
		rect.setSize(curstate.size);
		rect.setFillColor(ToColor(curstate.color_main));
		rect.setOutlineThickness(curstate.border_thickness);
		rect.setOutlineColor(ToColor(curstate.color_border));
		window->draw(rect);
	}

	sf::Vector2f thisone = this->used_view.getSize();
	sf::Vector2f view_size = default_view.getSize();
	sf::View gview = window->getView();
	sf::FloatRect global_view = sf::FloatRect(gview.getCenter() - gview.getSize()*0.5f, gview.getSize());
	sf::FloatRect this_view = overlap(global_view, sf::FloatRect(curstate.position, curstate.size));
	boxView.reset(this_view);
	sf::FloatRect global_viewport = gview.getViewport();
	sf::FloatRect local_viewport = sf::FloatRect(curstate.position.x / view_size.x, curstate.position.y / view_size.y, curstate.size.x / view_size.x, curstate.size.y / view_size.y);
	sf::FloatRect this_viewport = overlap(global_viewport, local_viewport);
	boxView.setViewport(this_viewport);
	
	float line_height = 0;
	float cur_shift_x1 = curstate.margin, cur_shift_x2 = curstate.margin;
	float cur_shift_y = curstate.margin + curstate.scroll;

	//update all the stuff inside the box
	for (auto &obj : objects)
	{
		Allign A = obj.get()->obj_allign;
		bool not_placed = true;
		int tries = 0;
		int obj_id = obj.get()->id;
		float obj_h = obj.get()->curstate.size.y;
		while (not_placed && tries < 2) //try to place the object somewhere
		{
			float space_left = defaultstate.size.x - cur_shift_x1 - cur_shift_x2;
			float obj_width = obj.get()->curstate.size.x;

			if (space_left >= obj_width || space_left >= defaultstate.size.x - 2 * curstate.margin)
			{
				not_placed = false;
				switch (A)
				{
				case LEFT:
					obj.get()->SetPosition(curstate.position.x + cur_shift_x1, curstate.position.y + cur_shift_y);
					cur_shift_x1 += obj_width + curstate.margin;
					line_height = std::max(obj_h, line_height);
					break;
				case CENTER:
					obj.get()->SetPosition(curstate.position.x + defaultstate.size.x * 0.5f - obj_width * 0.5f, curstate.position.y + cur_shift_y);
					line_height = std::max(obj_h, line_height);
					cur_shift_y += line_height + curstate.margin;
					line_height = 0;
					cur_shift_x1 = curstate.margin;
					cur_shift_x2 = curstate.margin;
					break;
				case RIGHT:
					obj.get()->SetPosition(curstate.position.x + defaultstate.size.x - obj_width - cur_shift_x2, curstate.position.y + cur_shift_y);
					cur_shift_x2 += obj_width + curstate.margin;
					line_height = std::max(obj_h, line_height);
					break;
				}
			}
			else
			{
				cur_shift_y += line_height + curstate.margin;
				line_height = 0;
				cur_shift_x1 = curstate.margin;
				cur_shift_x2 = curstate.margin;
				tries++;
			}

		}
		if (tries >= 2)
		{
			//ERROR_MSG("Object does not fit in the box.");
		}
		sf::FloatRect obj_box(obj.get()->curstate.position, obj.get()->curstate.size);
		sf::FloatRect seen_part = overlap(obj_box, this_view);
		//if the object is seen in the view, then update it
		if (seen_part.width > 0.f && seen_part.height > 0.f)
		{
			obj.get()->used_view = boxView;
			obj.get()->Update(window, state);
		}

	}

	cur_shift_y += line_height + curstate.margin;

	this->SetInsideSize(cur_shift_y - curstate.scroll - curstate.margin);

}

Box::Box(float x, float y, float dx, float dy, sf::Color color_main): auto_size(false)
{
	defaultstate.position.x = x;
	defaultstate.position.y = y;
	defaultstate.size.x = dx;
	defaultstate.size.y = dy;

	defaultstate.color_main = ToColorF(color_main);
	clone_states();
}

Box::Box(float dx, float dy, sf::Color color_main) : auto_size(false)
{
	defaultstate.position.x = 0;
	defaultstate.position.y = 0;
	defaultstate.size.x = dx;
	defaultstate.size.y = dy;
	defaultstate.color_main = ToColorF(color_main);
	clone_states();
}

Box::Box() : auto_size(false)
{ }

Box::Box(Box & A)
{
	*this = A;
}

Box::Box(Box && A)
{
	*this = A;
}

void Box::SetAutoSize(bool b)
{
	auto_size = b;
}

void Box::operator=(Box & A)
{
	copy(A);

	auto_size = A.auto_size;
	rect = A.rect;
	boxView = A.boxView;
	SetBackground(A.image);
}

void Box::operator=(Box && A)
{
	copy(A);

	std::swap(auto_size, A.auto_size);
	std::swap(image, A.image);
	std::swap(rect, A.rect);
	std::swap(boxView, A.boxView);
}

Object * Box::GetCopy()
{
	return static_cast<Object*>(new Box(*this));
}

Box::~Box()
{
	
}

ColorFloat::ColorFloat(float red, float green, float blue, float alpha): r(red), g(green), b(blue), a(alpha)
{

}

void ColorFloat::operator=(sf::Color a)
{
	*this = ToColorF(a);
}

void Text::Draw(sf::RenderWindow * window, InputState& state)
{
	if (text.get() != nullptr)
	{
		text.get()->setPosition(curstate.position);
		text.get()->setCharacterSize(curstate.font_size);
		text.get()->setFillColor(ToColor(curstate.color_main));
		text.get()->setOutlineThickness(curstate.border_thickness);
		text.get()->setOutlineColor(ToColor(curstate.color_border));
		window->draw(*text.get());
		SetSize(text.get()->getLocalBounds().width, text.get()->getLocalBounds().height);
	}
}

Text::Text(sf::Text t)
{
	text.reset(new sf::Text(t));
	defaultstate.font_size = t.getCharacterSize();
	defaultstate.color_main = t.getFillColor();
	clone_states();
}


Text::Text(Text & A)
{
	*this = A;
}

Text::Text(Text && A)
{
	*this = A;
}

void Text::operator=(Text & A)
{
	copy(A);
	if (A.text.get() != nullptr)
	{
		text.reset(new sf::Text(*A.text.get()));
	}
}

void Text::operator=(Text && A)
{
	copy(A);

	std::swap(text, A.text);
}

Object * Text::GetCopy()
{
	return static_cast<Object*>(new Text(*this));
}

void Window::Add(Object* something, Allign a)
{
	objects[1].get()->AddObject(something, a);
}

Window::Window(Window & A)
{
	*this = A;
}

Window::Window(Window && A)
{
	*this = A;
}

void Window::CreateCallbacks()
{
	//use lambda funtion
	this->objects[0].get()->objects[1].get()->SetMainCallbackFunction([parent = this](sf::RenderWindow * window, InputState & state)
	{
		Add2DeleteQueue(parent->id);
	});

	//delete callback
	this->SetMainDefaultFunction([parent = this](sf::RenderWindow * window, InputState & state)
	{
		if (state.key_press[sf::Keyboard::Escape] == true)
		{
			Add2DeleteQueue(parent->id);
			parent->action_time = action_dt;
		}
	});

	//drag callback
	this->objects[0].get()->SetMainCallbackFunction([parent = this](sf::RenderWindow * window, InputState & state)
	{
		parent->Move(state.mouse_speed);
	}, false);
}

void Window::operator=(Window & A)
{
	Box::operator=(A);
	CreateCallbacks();
}

void Window::operator=(Window && A)
{
	Box::operator=(A);
	CreateCallbacks();
}

Object * Window::GetCopy()
{
	return static_cast<Object*>(new Window(*this));
}

InputState::InputState(): mouse_pos(sf::Vector2f(0,0)), mouse_speed(sf::Vector2f(0, 0))
{
	std::fill(keys, keys + sf::Keyboard::KeyCount - 1, false);
	std::fill(mouse, mouse + 2, false);
}

InputState::InputState(bool a[sf::Keyboard::KeyCount], bool b[3], sf::Vector2f c, sf::Vector2f d):
	mouse_pos(c), mouse_speed(d)
{
	std::copy(a, a + sf::Keyboard::KeyCount - 1, keys);
	std::copy(b, b + 2, mouse);
}

void MenuBox::AddObject(Object * something, Allign a)
{
	this->objects[0].get()->AddObject(something, a);

	//update the slider
	float inside_size = this->objects[0].get()->defaultstate.inside_size;
	float height = this->objects[0].get()->defaultstate.size.y;
	float height_1 = height - 2 * this->objects[1].get()->defaultstate.margin;
	float new_h = std::min(height_1 * (height / inside_size), height_1);
	if (new_h == height_1 || auto_size)
	{
		//remove the slider
		this->objects[1].get()->SetWidth(0);
		this->objects[1].get()->objects[0].get()->SetWidth(0);
		this->objects[0].get()->SetWidth(this->defaultstate.size.x);
		this->objects[1].get()->SetHeigth(0);
	}
	else
	{
		this->objects[0].get()->SetWidth(this->defaultstate.size.x - 40);
		this->objects[1].get()->SetWidth(30);
		this->objects[1].get()->objects[0].get()->SetWidth(26);
		this->objects[1].get()->objects[0].get()->SetHeigth(new_h);
		this->objects[1].get()->SetHeigth(height);
	}
}

void MenuBox::Cursor(int d)
{
	if (d == -1) // up
	{
		if (cursor_id > 0)
		{
			cursor_id--;
		}
	}
	else if(d == 1)
	{
		if (cursor_id < this->objects[0].get()->objects.size() - 1)
		{
			cursor_id++;	
		}
	}
	cursor = this->objects[0].get()->objects[cursor_id].get()->id;

	//apply scroll to chosen object
	float ybox = this->curstate.position.y;
	float yobj = this->objects[0].get()->objects[cursor_id].get()->curstate.position.y;
	float dy = yobj - ybox;
	ScrollTo(dy);
}

void MenuBox::ScrollBy(float dx)
{
	float inside_size = this->objects[0].get()->defaultstate.inside_size;
	float cur_scroll = -this->objects[0].get()->defaultstate.scroll + dx;
	float height_1 = this->objects[0].get()->defaultstate.size.y - 2 * this->objects[1].get()->defaultstate.margin;
	float height_2 = this->objects[1].get()->objects[0].get()->defaultstate.size.y;
	//only scroll within the appropriate range
	if (cur_scroll <= inside_size - height_1 && cur_scroll >= 0 && inside_size > height_1)
	{
		this->objects[0].get()->ApplyScroll(-cur_scroll);
		float max_slide_scroll = height_1 - height_2;
		//relative scroll of the scroll button
		float scroll_a = max_slide_scroll * cur_scroll / (inside_size - height_1);
		this->objects[1].get()->SetScroll(scroll_a);
	}
}

void MenuBox::ScrollTo(float scroll)
{
	float cur_scroll = this->objects[0].get()->defaultstate.scroll;
	float ds = scroll - this->objects[0].get()->defaultstate.margin;
	ScrollBy(ds);
}

MenuBox::MenuBox(float dx, float dy, bool auto_y, float x, float y, sf::Color color_main) : cursor_id(0)
{
	SetAutoSize(auto_y);
	defaultstate.position.x = x;
	defaultstate.position.y = y;
	defaultstate.size.x = dx;
	defaultstate.size.y = dy;
	defaultstate.color_main = ToColorF(color_main);
	clone_states();
	float scroll_size = auto_y?0:30;
	Box Inside(0, 0, dx - scroll_size, dy, sf::Color(100, 100, 100, 128)),
		Scroll(0, 0, scroll_size, auto_y ? 0 : dy, sf::Color(150, 150, 150, 128)),
		Scroll_Slide(0, 0, scroll_size-2, 60, sf::Color(255, 150, 0, 128));

	Inside.SetAutoSize(auto_y);
	Scroll_Slide.hoverstate.color_main = sf::Color(255, 50, 0, 128);
	Scroll_Slide.activestate.color_main = sf::Color(255, 100, 100, 255);
	Inside.SetBackgroundColor(sf::Color::Transparent);
	Scroll.SetMargin(2);
	Inside.SetMargin(12);
	Scroll.AddObject(&Scroll_Slide, Box::CENTER);
	this->Object::AddObject(&Inside, Box::LEFT);
	this->Object::AddObject(&Scroll, Box::RIGHT);

	CreateCallbacks();
}

MenuBox::MenuBox(MenuBox & A): cursor_id(0)
{
	*this = A;
}

MenuBox::MenuBox(MenuBox && A): cursor_id(0)
{
	*this = A;
}

void MenuBox::CreateCallbacks()
{
	//use lambda funtion
	this->objects[1].get()->objects[0].get()->SetMainCallbackFunction([parent = this](sf::RenderWindow * window, InputState & state)
	{
		float inside_size = parent->objects[0].get()->defaultstate.inside_size;
		float height_1 = parent->objects[1].get()->defaultstate.size.y - 2 * parent->objects[1].get()->defaultstate.margin;
		float height_2 = parent->objects[1].get()->objects[0].get()->defaultstate.size.y;
		float max_slide_scroll = height_1 - height_2;
		//relative scroll coefficient
		float rel_coef = (inside_size - height_1) / max_slide_scroll;
		parent->ScrollBy(state.mouse_speed.y*rel_coef);
	}, false);

	this->SetMainHoverFunction([parent = this](sf::RenderWindow * window, InputState & state)
	{
		//wheel scroll 
		if (state.wheel != 0.f)
		{
			float ds = 20.f;
			parent->ScrollBy(-state.wheel*ds);
		}
	});

	this->SetMainDefaultFunction([parent = this](sf::RenderWindow * window, InputState & state)
	{
		bool A = false;

		if (state.keys[sf::Keyboard::Up])
		{
			parent->Cursor(-1);
			A = 1;	
		}

		if (state.keys[sf::Keyboard::Down])
		{
			parent->Cursor(1);
			A = 1;
		}

		if (state.keys[sf::Keyboard::Return])
		{
			//run the callback function of the chosen object
			A = parent->objects[0].get()->objects[parent->cursor_id].get()->RunCallback(window, state);
		}

		if (A) parent->action_time = action_dt;

		A = false;

		if (state.key_press[sf::Keyboard::Up])
		{
			parent->action_time = action_dt / 4;
		}

		if (state.key_press[sf::Keyboard::Down])
		{
			parent->action_time = action_dt / 4;
		}
	});
}

void MenuBox::operator=(MenuBox & A)
{
	Box::operator=(A);
	CreateCallbacks();
}

void MenuBox::operator=(MenuBox && A)
{
	Box::operator=(A);
	CreateCallbacks();
}

Object * MenuBox::GetCopy()
{
	return static_cast<Object*>(new MenuBox(*this));
}

Button::Button(Button & A)
{
	*this = A;
}

Button::Button(Button && A)
{
	*this = A;
}

void Button::operator=(Button & A)
{
	Box::operator=(A);
}

void Button::operator=(Button && A)
{
	Box::operator=(A);
}

Object * Button::GetCopy()
{
	return static_cast<Object*>(new Button(*this));
}

Button::~Button()
{
}

Image::Image(sf::Texture image, float w, float h, sf::Color color_hover)
{
	SetSize((w == 0) ? image.getSize().x : w, (h == 0) ? image.getSize().y : h);

	SetBackgroundColor(sf::Color::White);
	hoverstate.color_main = color_hover;

	this->SetBackground(image);
}

Image::Image(std::string image_path, float w, float h, sf::Color color_hover)
{
	sf::Texture image;
	image.loadFromFile(image_path);

	SetSize((w == 0) ? image.getSize().x : w, (h == 0) ? image.getSize().y : h);
	SetBackgroundColor(sf::Color::White);
	hoverstate.color_main = color_hover;

	this->SetBackground(image);
}

Image::Image(Image & A)
{
	*this = A;
}

Image::Image(Image && A)
{
	*this = A;
}

void Image::operator=(Image & A)
{
	Box::operator=(A);
}

void Image::operator=(Image && A)
{
	Box::operator=(A);
}

Object * Image::GetCopy()
{
	return static_cast<Object*>(new Image(*this));
}

Image::~Image()
{
}

KeyMapper::KeyMapper(KeyMapper & A)
{
	*this = A;
}

KeyMapper::KeyMapper(KeyMapper && A)
{
	*this = A;
}

void KeyMapper::operator=(KeyMapper & A)
{
	Box::operator=(A);
	this_type = A.this_type;
	key_ptr = A.key_ptr;
	waiting = A.waiting;
	wait_text = A.wait_text;
	CreateCallbacks();
}

void KeyMapper::operator=(KeyMapper && A)
{
	Box::operator=(A);
	std::swap(this_type, A.this_type);
	std::swap(key_ptr, A.key_ptr);
	waiting = A.waiting;
	wait_text = A.wait_text;
	CreateCallbacks();
}

void KeyMapper::CreateCallbacks()
{
	//use a lambda function
	this->objects[1].get()->SetMainCallbackFunction([parent = this](sf::RenderWindow * window, InputState & state)
	{
		parent->waiting = true;
		//get the text pointer out of the object pointer inside the parent
		Text *text_ptr = static_cast<Text*>(parent->objects[1].get()->objects[0].get());
		text_ptr->SetString(parent->wait_text);
	});


	this->SetMainDefaultFunction([parent = this](sf::RenderWindow * window, InputState & state)
	{
		if (parent->waiting)
		{
			if (state.key_press[sf::Keyboard::Escape])
			{
				parent->SetKeyString(); //exit
			}
			else switch (parent->this_type)
			{
			case KEYBOARD:
				if (state.isKeyPressed)
				{
					//search for the pressed key
					for (sf::Keyboard::Key i = sf::Keyboard::A; i < sf::Keyboard::KeyCount; i = sf::Keyboard::Key(i + 1))
					{
						if (state.keys[i]) //found
						{
							*(parent->key_ptr) = i;
							parent->SetKeyString();
							break;
						}
					}
				}
				break;
			case JOYSTICK_AXIS:
				//search for the axis
				for (int i = 0; i < sf::Joystick::AxisCount; i++)
				{
					if (abs(state.axis_value[i]) > 0.5f  && state.axis_moved[i])
					{
						*(parent->key_ptr) = i;
						parent->SetKeyString();
						break;
					}
				}
				break;
			case JOYSTICK_KEYS:
				//search for the pressed joystick key
				for (int i = 0; i < sf::Joystick::ButtonCount; i++)
				{
					if (state.button_pressed[i])
					{
						*(parent->key_ptr) = i;
						parent->SetKeyString();
						break;
					}
				}
				
				break;
			}
		}
	});
}

void KeyMapper::SetKeyString()
{
	Text *text_ptr = static_cast<Text*>(objects[1].get()->objects[0].get());
	waiting = false;
	sf::Keyboard::Key key = sf::Keyboard::Key(*key_ptr);
	switch (this_type)
	{
	case KEYBOARD:
		text_ptr->SetString(key_name(key));
		break;
	case JOYSTICK_AXIS:
		text_ptr->SetString("Axis " + num2str(*key_ptr));
		break;
	case JOYSTICK_KEYS:
		text_ptr->SetString("Key " + num2str(*key_ptr));
		break;
	}
}

Object * KeyMapper::GetCopy()
{
	return static_cast<Object*>(new KeyMapper(*this));
}
