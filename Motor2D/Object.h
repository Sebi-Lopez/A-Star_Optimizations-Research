#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "p2Point.h"
#include "SDL/include/SDL_rect.h"

enum class ObjectType {
	WALL,
	START,
	END,
	MAX
};

class Object 
{

public: 

	Object(const iPoint& pos); 
	~Object(); 


	virtual bool Update(float dt);
	virtual bool Draw() ; 

private:
	bool InsideMe(const iPoint& pos) const;

public: 
	bool to_delete = false;
	ObjectType type = ObjectType::MAX;

private: 
	iPoint position;
	iPoint tilePos; 
	SDL_Rect rect;
};



#endif
