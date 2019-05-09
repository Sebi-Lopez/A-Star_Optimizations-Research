#ifndef __OBJECTFACTORY_H__
#define __OBJECTFACTORY_H__

#include "j1Module.h"
#include "p2Point.h"
#include "Object.h"

#include <vector>

struct Object; 


class ObjectFactory : public j1Module
{

public: 

	ObjectFactory(); 
	~ObjectFactory();

	bool PreUpdate() override; 
	bool Update(float dt) override; 
	bool PostUpdate() override; 

	Object* CreateObject(const iPoint& pos, ObjectType type);

private: 

	std::vector<Object*> objects;
};


#endif 