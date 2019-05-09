#include "ObjectFactory.h"
#include "j1App.h"
#include "j1Input.h"
#include "j1Render.h"
#include "j1Map.h"
#include "p2Log.h"

ObjectFactory::ObjectFactory()
{
}

ObjectFactory::~ObjectFactory()
{
}

bool ObjectFactory::PreUpdate()
{
	int x, y; 
	App->input->GetMousePosition(x, y); 
	iPoint mousePos = App->render->ScreenToWorld(x, y);
	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT)
	{
		CreateObject(mousePos, ObjectType::WALL);
		LOG("Creating Object at: %i, %i", mousePos.x, mousePos.y); 
	}

	return true; 
}

bool ObjectFactory::Update(float dt)
{

	bool ret = true; 

	std::vector<Object*>::iterator item = objects.begin();
	for (; item != objects.end();)
	{
		if ((*item) != nullptr)
		{
			if (!(*item)->to_delete)
			{
				ret = (*item)->Update(dt); 
				++item; 
			}
			else 
			{
				delete(*item);
				(*item) = nullptr;
				item = objects.erase(item);
			}
		}
	}	

	return ret; 
}

bool ObjectFactory::PostUpdate()
{
	bool ret = true; 

	std::vector<Object*>::iterator item = objects.begin();
	for (; item != objects.end(); ++item)
	{
		if ((*item) != nullptr)
		{
			ret = (*item)->Draw();
		}
	}
	return ret;
}

Object * ObjectFactory::CreateObject(const iPoint & pos, ObjectType type)
{
	Object* ret = nullptr; 
	iPoint position = App->map->WorldToMap(pos.x, pos.y);

	ret = new Object(position); 
	
	if (ret != nullptr)
	{
		ret->type = type; 
		objects.push_back(ret); 
	}

	
	return ret;
}
