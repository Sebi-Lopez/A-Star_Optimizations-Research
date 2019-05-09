#include "Object.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Input.h"
#include "j1Map.h"

Object::Object(const iPoint& pos) : position(App->map->MapToWorld(pos.x, pos.y)), rect({position.x, position.y, App->map->data.tile_width, App->map->data.tile_height})
{
	//tilePos = App->map->WorldToMap(position.x, position.y);
}

Object::~Object()
{
}

bool Object::Update(float dt)
{
	int x, y; 
	App->input->GetMousePosition(x, y);
	iPoint mousePos = App->render->ScreenToWorld(x, y);

	if (InsideMe(mousePos))
	{
		/*if (App->input->GetMouseButtonDown((SDL_BUTTON_LEFT) == KEY_REPEAT))
			to_delete = true; */
	}

	return true;
}

bool Object::Draw()
{
	switch (type)
	{
	case ObjectType::WALL:
		App->render->DrawQuad(rect, 0,0,255, 100, true);
		break; 

	case ObjectType::START:
		App->render->DrawQuad(rect, 255, 0, 0, 100, true);
		break;

	case ObjectType::END:
		App->render->DrawQuad(rect, 0, 255, 0, 100, true);
		break;

	case ObjectType::MAX:
		break; 

	default: 
		break; 
	}
	return true;
}

bool Object::InsideMe(const iPoint& pos) const
{
	return (pos.x > rect.x && pos.x < rect.x + rect.w 
		&& pos.y > rect.y && pos.y < rect.y + rect.h);
}
