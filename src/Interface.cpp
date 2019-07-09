#include <Interface.h>

std::map<int, std::unique_ptr<Object>> global_objects;
std::map<int, int> z_value; //rendering order
std::vector<int> del;
int obj_id = 0;

sf::Color default_main_color = sf::Color(128, 128, 128, 128);
sf::Color default_hover_main_color = sf::Color(200, 128, 128, 128);
sf::Color default_active_main_color = sf::Color(255, 128, 128, 255);
float default_margin =0;
sf::View default_view = sf::View(sf::FloatRect(0, 0, 1600, 900));

float animation_sharpness = 4.f;
float action_dt = 0.2;

static const std::string close_png = "images/clear.png";

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

void AddGlobalObject(Object & a)
{
	global_objects[obj_id] = std::unique_ptr<Object>(a.GetCopy());//add a copy to the global list
	global_objects[obj_id].get()->id = obj_id;
	obj_id++;
}

void RemoveGlobalObject(int id)
{
	global_objects.erase(id);
}

void Add2DeleteQueue(int id)
{
	del.push_back(id);
}

void UpdateAllObjects(sf::RenderWindow * window, InputState& state)
{
	for (auto &id : del)
	{
		RemoveGlobalObject(id);
	}
	del.clear();
	for (auto &obj : global_objects)
	{
		obj.second->Update(window, state);
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
}

void Object::SetBorderColor(sf::Color color)
{
	
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
	defaultstate.position += dx;
	curstate.position += dx;
	activestate.position += dx;
	hoverstate.position += dx;
}

void Object::SetDefaultFunction(std::function<void(sf::RenderWindow * window, InputState&state)> fun)
{
	defaultfn = fun;
}

void Object::SetCallbackFunction(std::function<void(sf::RenderWindow * window, InputState&state)> fun)
{
	callback = fun;
}

void Object::SetHoverFunction(std::function<void(sf::RenderWindow * window, InputState&state)> fun)
{
	hoverfn = fun;
}

void Object::clone_states()
{
	curstate = defaultstate;
	activestate = defaultstate;
	hoverstate = defaultstate;
}

void Object::Draw(sf::RenderWindow * window, InputState& state)
{
	//nothing to draw
}

void Object::Update(sf::RenderWindow * window, InputState& state)
{
	window->setView(used_view);
	//callback stuff
	sf::Vector2f worldPos = window->mapPixelToCoords(sf::Vector2i(state.mouse_pos.x, state.mouse_pos.y));
	sf::FloatRect obj(curstate.position.x, curstate.position.y, curstate.size.x, curstate.size.y);

	state.mouse_speed = window->mapPixelToCoords(sf::Vector2i(state.mouse_pos.x, state.mouse_pos.y)) -
						window->mapPixelToCoords(sf::Vector2i(state.mouse_prev.x, state.mouse_prev.y));

	curmode = DEFAULT;
	//if mouse is inside the object 
	if (obj.contains(worldPos))
	{
		if (state.mouse[0] || state.mouse[2]) //if click
		{
			if (callback != NULL)
				callback(window, state); //run callback with state info
			curmode = ACTIVE;
		}
		else // if hover
		{
			if (hoverfn != NULL)
				hoverfn(window, state); //run callback with state info
			curmode = ONHOVER;
		}
	}
	else curmode = DEFAULT;

	if (action_time < 0.f)
	{
		if (defaultfn != NULL)
			defaultfn(window, state); //run callback with state info
	}
	else
	{
		action_time -= state.dt;
	}

	float t = 1.f - exp(-animation_sharpness * state.dt);

	//animation
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


Object::Object() : callback(NULL), hoverfn(NULL), defaultfn(NULL), curmode(DEFAULT), action_time(0.2), obj_allign(LEFT)
{
	curstate.color_main = default_main_color;
	activestate.color_main = default_active_main_color;
	hoverstate.color_main = default_hover_main_color;
	defaultstate.color_main = default_main_color;

	used_view = default_view;
}

Object::~Object()
{
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
	activestate = A.activestate;
	hoverstate = A.hoverstate;
	defaultstate = A.defaultstate;
	curmode = A.curmode;
	used_view = A.used_view;
	obj_allign = A.obj_allign;
	id = A.id;
	action_time = A.action_time;

	for (auto &element : A.objects)
		AddObject(element.get(), element.get()->obj_allign);
}

void Object::copy(Object && A)
{
	std::swap(curstate, A.curstate);
	std::swap(activestate, A.activestate);
	std::swap(hoverstate, A.hoverstate);
	std::swap(defaultstate, A.defaultstate);
	std::swap(curmode, A.curmode);
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

void Object::AddObject(Object * something, Allign a)
{
	something->obj_allign = a;
	objects.push_back(std::unique_ptr<Object>(something->GetCopy()));
	this->SetInsideSize(defaultstate.inside_size + something->defaultstate.size.y);
}

void Box::SetBackground(const sf::Texture & texture)
{
	image = texture;
	rect.setTexture(&image);
}

void Box::Draw(sf::RenderWindow * window, InputState& state)
{
	//update the box itself
	rect.setPosition(curstate.position);
	rect.setSize(curstate.size);
	rect.setFillColor(ToColor(curstate.color_main));
	rect.setOutlineThickness(curstate.border_thickness);
	rect.setOutlineColor(ToColor(curstate.color_border));
	window->draw(rect);
	
	sf::Vector2f default_size = sf::Vector2f(1600, 900);
	sf::FloatRect global_view = sf::FloatRect(window->getView().getCenter() - window->getView().getSize()*0.5f, window->getView().getSize());
	sf::FloatRect this_view = overlap(global_view, sf::FloatRect(curstate.position, curstate.size));
	boxView.reset(this_view);
	sf::FloatRect global_viewport = window->getView().getViewport();
	sf::FloatRect local_viewport = sf::FloatRect(curstate.position.x / default_size.x, curstate.position.y / default_size.y, curstate.size.x / default_size.x, curstate.size.y / default_size.y);
	sf::FloatRect this_viewport = overlap(global_viewport, local_viewport);
	boxView.setViewport(this_viewport);

	if (this_viewport.width > 0.f && this_viewport.height > 0.f)
	{
		float line_height = 0;
		float cur_shift_x1 = curstate.margin, cur_shift_x2 = curstate.margin;
		float cur_shift_y = curstate.margin + curstate.scroll;

		//update all the stuff inside the box
		for (auto &obj : objects)
		{
			Allign A = obj.get()->obj_allign;
			bool not_placed = true;
			int tries = 0;
			obj.get()->used_view = boxView;
			line_height = std::max(obj.get()->curstate.size.y, line_height);
			while (not_placed && tries < 2) //try to place the object somewhere
			{
				float space_left = defaultstate.size.x - cur_shift_x1 - cur_shift_x2;
				float obj_width = obj.get()->curstate.size.x;

				if (space_left >= obj_width)
				{
					not_placed = false;
					switch (A)
					{
					case LEFT:
						obj.get()->SetPosition(curstate.position.x + cur_shift_x1, curstate.position.y + cur_shift_y);
						cur_shift_x1 += obj_width + curstate.margin;
						break;
					case CENTER:
						obj.get()->SetPosition(curstate.position.x + defaultstate.size.x * 0.5f - obj_width * 0.5f, curstate.position.y + cur_shift_y);
						cur_shift_y += line_height + curstate.margin;
						line_height = 0;
						cur_shift_x1 = curstate.margin;
						cur_shift_x2 = curstate.margin;
						break;
					case RIGHT:
						obj.get()->SetPosition(curstate.position.x + defaultstate.size.x - obj_width - cur_shift_x2, curstate.position.y + cur_shift_y);
						cur_shift_x2 += obj_width + curstate.margin;
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

			obj.get()->Update(window, state);
		}
		this->SetInsideSize(cur_shift_y - curstate.scroll - curstate.margin);
	}
}

Box::Box(float x, float y, float dx, float dy, sf::Color color_main)
{
	defaultstate.position.x = x;
	defaultstate.position.y = y;
	defaultstate.size.x = dx;
	defaultstate.size.y = dy;
	defaultstate.color_main = ToColorF(color_main);
	clone_states();
}

Box::Box()
{

}

Box::Box(Box & A)
{
	*this = A;
}

Box::Box(Box && A)
{
	*this = A;
}

void Box::operator=(Box & A)
{
	copy(A);

	rect = A.rect;
	boxView = A.boxView;
	SetBackground(A.image);
}

void Box::operator=(Box && A)
{
	copy(A);

	std::swap(image, A.image);
	std::swap(rect, A.rect);
	std::swap(boxView, A.boxView);
}

Object * Box::GetCopy()
{
	return static_cast<Object*>(new Box(*this));
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
	text.setPosition(curstate.position);
	text.setCharacterSize(curstate.font_size);
	text.setFillColor(ToColor(curstate.color_main));
	text.setOutlineThickness(curstate.border_thickness);
	text.setOutlineColor(ToColor(curstate.color_border));

	window->draw(text);

	SetSize(text.getLocalBounds().width, text.getLocalBounds().height);
}

Text::Text(std::string str, sf::Font & f, float size, sf::Color col)
{
	text.setString(str);
	text.setFont(f);
	defaultstate.font_size = size;
	defaultstate.color_main = col;

	clone_states();
}

Text::Text(sf::Text t)
{
	text = t;
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

	text = A.text;
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

	//update the slider
	float inside_size = this->objects[1].get()->defaultstate.inside_size;
	float height = this->objects[2].get()->defaultstate.size.y;
	float height_1 = height - 2 * this->objects[2].get()->defaultstate.margin;
	float new_h = std::min(height_1 * (height / inside_size), height_1);
	this->objects[2].get()->objects[0].get()->SetHeigth(new_h);
}

void Window::ScrollBy(float dx)
{
	float inside_size = this->objects[1].get()->defaultstate.inside_size;
	float cur_scroll = -this->objects[1].get()->defaultstate.scroll + dx;
	if (cur_scroll <= inside_size && cur_scroll >= 0)
	{
		this->objects[1].get()->ApplyScroll(-cur_scroll);

		float height_1 = this->objects[2].get()->defaultstate.size.y - 2 * this->objects[2].get()->defaultstate.margin;
		float height_2 = this->objects[2].get()->objects[0].get()->defaultstate.size.y;
		float max_slide_scroll = height_1 - height_2;
		//relative scroll
		float scroll_a = max_slide_scroll * cur_scroll / inside_size;
		this->objects[2].get()->SetScroll(scroll_a);
	}
}

Window::Window(float x, float y, float dx, float dy, sf::Color color_main, std::string title, sf::Font & font)
{
	defaultstate.position.x = x;
	defaultstate.position.y = y;
	defaultstate.size.x = dx;
	defaultstate.size.y = dy;
	defaultstate.color_main = ToColorF(color_main);
	clone_states();

	Box Bar(0, 0, dx, 30, sf::Color(0, 100, 200, 128)), 
		CloseBx(0, 0, 30, 30, sf::Color(255, 255, 255, 255)),
		Inside(0, 0, dx - 30, dy - 30, sf::Color(100, 100, 100, 128)),
		Scroll(0, 0, 30, dy - 30, sf::Color(150, 150, 150, 128)),
		Scroll_Slide(0, 0, 27, 60, sf::Color(255, 150, 0, 128));
	Text Title(title, font, 25, sf::Color::White);

	Scroll_Slide.hoverstate.color_main = sf::Color(255, 50, 0, 128);
	Scroll_Slide.activestate.color_main = sf::Color(255, 100, 100, 255);

	sf::Image close; close.loadFromFile(close_png);
	sf::Texture closetxt; closetxt.loadFromImage(close);
	CloseBx.SetBackground(closetxt);

	Bar.AddObject(&Title, Box::LEFT);
	Bar.AddObject(&CloseBx, Box::RIGHT);
	Scroll.AddObject(&Scroll_Slide, Box::CENTER);
	this->AddObject(&Bar, Box::CENTER);
	this->AddObject(&Inside, Box::LEFT);
	this->AddObject(&Scroll, Box::RIGHT);

	CreateCallbacks();
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
	this->objects[2].get()->objects[0].get()->SetCallbackFunction([parent = this](sf::RenderWindow * window, InputState & state)
	{
		parent->ScrollBy(state.mouse_speed.y);
	});

	//use lambda funtion
	this->objects[0].get()->objects[1].get()->SetCallbackFunction([parent = this](sf::RenderWindow * window, InputState & state)
	{
		Add2DeleteQueue(parent->id);
	});

	//drag callback
	this->objects[0].get()->SetCallbackFunction([parent = this](sf::RenderWindow * window, InputState & state)
	{
		parent->Move(state.mouse_speed);
	});

	this->SetHoverFunction([parent = this](sf::RenderWindow * window, InputState & state)
	{
		//wheel scroll 
		if (state.wheel != 0.f)
		{
			float ds = 20.f;
			parent->ScrollBy(state.wheel*ds);
		}
	});
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

void MenuBox::Add(Object * something, Allign a)
{
	Inside.get()->AddObject(something, a);
}

void MenuBox::Cursor(int d)
{
	if (d == -1) // up
	{
		if (cursor_id > 0)
		{
			cursor_id--;
			Inside.get()->objects[cursor_id].get()->curmode = ONHOVER;
		}
	}
	else if(d == 1)
	{
		if (cursor_id < Inside.get()->objects.size() - 1)
		{
			cursor_id++;
			Inside.get()->objects[cursor_id].get()->curmode = ONHOVER;
		}
	}
}

void MenuBox::ScrollBy(float dx)
{
	float scroll_a = this->Scroll.get()->defaultstate.scroll + dx;
	float scroll_b = this->Inside.get()->defaultstate.scroll - dx;
	this->Inside.get()->ApplyScroll(scroll_b);
	this->Scroll.get()->SetScroll(scroll_a);
}

MenuBox::MenuBox(float x, float y, float dx, float dy, sf::Color color_main, std::string title, sf::Font & font): cursor_id(0)
{
	defaultstate.position.x = x;
	defaultstate.position.y = y;
	defaultstate.size.x = dx;
	defaultstate.size.y = dy;
	defaultstate.color_main = ToColorF(color_main);
	clone_states();

	Box Inside(0, 0, dx - 30, dy - 30, sf::Color(100, 100, 100, 128)),
		Scroll(0, 0, 30, dy - 30, sf::Color(150, 150, 150, 128)),
		Scroll_Slide(0, 0, 27, 60, sf::Color(255, 150, 0, 128));
	Text Title(title, font, 25, sf::Color::White);

	Scroll_Slide.hoverstate.color_main = sf::Color(255, 50, 0, 128);
	Scroll_Slide.activestate.color_main = sf::Color(255, 100, 100, 255);

	Scroll.AddObject(&Scroll_Slide, Box::CENTER);
	this->AddObject(&Inside, Box::LEFT);
	this->AddObject(&Scroll, Box::RIGHT);

	CreateCallbacks();
}

MenuBox::MenuBox(MenuBox & A)
{
	*this = A;
}

MenuBox::MenuBox(MenuBox && A)
{
	*this = A;
}

void MenuBox::CreateCallbacks()
{
	//use lambda funtion
	this->objects[1].get()->objects[0].get()->SetCallbackFunction([parent = this](sf::RenderWindow * window, InputState & state)
	{
		parent->ScrollBy(state.mouse_speed.y);
	});

	this->SetHoverFunction([parent = this](sf::RenderWindow * window, InputState & state)
	{
		//wheel scroll 
		if (state.wheel != 0.f)
		{
			float ds = 20.f;
			parent->ScrollBy(state.wheel*ds);
		}
	});

	this->SetDefaultFunction([parent = this](sf::RenderWindow * window, InputState & state)
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

		if (state.keys[sf::Keyboard::Enter])
		{
			//run the callback function of the chosen object
			parent->Inside.get()->objects[parent->cursor_id].get()->callback(window, state);
			A = 1;
		}

		if (A) parent->action_time = action_dt;
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