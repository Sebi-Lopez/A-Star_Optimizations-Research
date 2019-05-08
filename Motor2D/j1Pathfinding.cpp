#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1PathFinding.h"
#include "j1Render.h"
#include "j1Map.h"
#include "j1Textures.h"

j1PathFinding::j1PathFinding() : j1Module(), map(NULL), last_path(DEFAULT_PATH_LENGTH),width(0), height(0)
{
	name.create("pathfinding");
}

// Destructor
j1PathFinding::~j1PathFinding()
{
	RELEASE_ARRAY(map);
}

bool j1PathFinding::Start()
{
	debugPath = App->tex->Load("maps/debugTex2.png");
	return true;
}

bool j1PathFinding::PostUpdate()
{
	iPoint pos = { 0,0 };
	std::list<PathNode>::iterator iterator = open.pathNodeList.begin(); 
	for (iterator; iterator != open.pathNodeList.end(); iterator++)
	{
		pos = App->map->MapToWorld((*iterator).pos.x, (*iterator).pos.y);		// X + 1, Same problem with map
		//App->render->Blit(debugPath, pos.x, pos.y);
		App->render->DrawQuad({ pos.x,pos.y,App->map->data.tile_height, App->map->data.tile_height }, 0, 255, 0, 255);
	}

	iterator = closed.pathNodeList.begin();
	for (iterator; iterator != closed.pathNodeList.end(); iterator++)
	{
		pos = App->map->MapToWorld((*iterator).pos.x, (*iterator).pos.y);		// X + 1, Same problem with map
		//App->render->Blit(debugPath, pos.x, pos.y);
		App->render->DrawQuad({ pos.x,pos.y,App->map->data.tile_height, App->map->data.tile_height }, 0, 0, 255, 100);
	}

	pos = App->map->MapToWorld(origin.x, origin.y);
	//App->render->Blit(debugPath, pos.x, pos.y);
	App->render->DrawQuad({ pos.x,pos.y,App->map->data.tile_height, App->map->data.tile_height }, 0, 0, 255, 255);


	pos = App->map->MapToWorld(goal.x, goal.y);
	//App->render->Blit(debugPath, pos.x, pos.y);
	App->render->DrawQuad({ pos.x,pos.y,App->map->data.tile_height, App->map->data.tile_height }, 255, 0, 0, 255);

	return true;
}

// Called before quitting
bool j1PathFinding::CleanUp()
{
	LOG("Freeing pathfinding library");

	last_path.clear();
	RELEASE_ARRAY(map);
	return true;
}

// Sets up the walkability map
void j1PathFinding::SetMap(uint width, uint height, uchar* data)
{
	this->width = width;
	this->height = height;

	RELEASE_ARRAY(map);
	map = new uchar[width*height];
	memcpy(map, data, width*height);
}

// Utility: return true if pos is inside the map boundaries
bool j1PathFinding::CheckBoundaries(const iPoint& pos) const
{
	return (pos.x >= 0 && pos.x <= (int)width &&
			pos.y >= 0 && pos.y <= (int)height);
}

// Utility: returns true is the tile is walkable
bool j1PathFinding::IsWalkable(const iPoint& pos) const
{
	uchar t = GetTileAt(pos);
	return t != INVALID_WALK_CODE && t > 0;
}

// Utility: return the walkability value of a tile
uchar j1PathFinding::GetTileAt(const iPoint& pos) const
{
	if(CheckBoundaries(pos))
		return map[(pos.y*width) + pos.x];

	return INVALID_WALK_CODE;
}

// To request all tiles involved in the last generated path
const std::vector<iPoint>* j1PathFinding::GetLastPath() const
{
	return &last_path;
}

// PathList ------------------------------------------------------------------------
// Looks for a node in this list and returns it's list node or NULL
// ---------------------------------------------------------------------------------
const PathNode* PathList::Find(const iPoint& point) const
{
	std::list<PathNode>::const_iterator item = pathNodeList.begin();

	while (item != pathNodeList.end())
	{
		if ((*item).pos == point)
			return &(*item);
		item++;
	}

	return nullptr;
}

// PathList ------------------------------------------------------------------------
// Returns the Pathnode with lowest score in this list or NULL if empty
// ---------------------------------------------------------------------------------
const PathNode* PathList::GetNodeLowestScore() const
{	
	const PathNode* ret = nullptr;
	int min = INT_MAX;

	std::list<PathNode>::const_reverse_iterator item = pathNodeList.rbegin();

	while (item != pathNodeList.rend())
	{
		if ((*item).Score() < min)
		{
			min = (*item).Score();
			ret = &(*item);
		}
		item++;
	}
	return ret;
}

// PathNode -------------------------------------------------------------------------
// Convenient constructors
// ----------------------------------------------------------------------------------
PathNode::PathNode() : g(-1), h(-1), pos(-1, -1), parent(nullptr), direction(0,0)
{}

PathNode::PathNode(int g, int h, const iPoint& pos, const PathNode* parent, const iPoint& direction, bool isDiagonal) : g(g), h(h), pos(pos), parent(parent), direction(direction), isDiagonal(isDiagonal)
{}

PathNode::PathNode(const PathNode& node) : g(node.g), h(node.h), pos(node.pos), parent(node.parent), direction(node.direction)
{}

// PathNode -------------------------------------------------------------------------
// Fills a list (PathList) of all valid adjacent pathnodes
// ----------------------------------------------------------------------------------
uint PathNode::FindWalkableAdjacents(PathList& list_to_fill) const
{
	iPoint cell;

	cell.create(pos.x, pos.y + 1);
	if (App->pathfinding->IsWalkable(cell))
		list_to_fill.pathNodeList.push_back(PathNode(-1, -1, cell, this));

	// south
	cell.create(pos.x, pos.y - 1);
	if (App->pathfinding->IsWalkable(cell))
		list_to_fill.pathNodeList.push_back(PathNode(-1, -1, cell, this));

	// east
	cell.create(pos.x + 1, pos.y);
	if (App->pathfinding->IsWalkable(cell))
		list_to_fill.pathNodeList.push_back(PathNode(-1, -1, cell, this));

	// west
	cell.create(pos.x - 1, pos.y);
	if (App->pathfinding->IsWalkable(cell))
		list_to_fill.pathNodeList.push_back(PathNode(-1, -1, cell, this));

	// north-west
	cell.create(pos.x + 1, pos.y - 1);
	if (App->pathfinding->IsWalkable(cell))
		list_to_fill.pathNodeList.push_back(PathNode(-1, -1, cell, this, { 0,0 }, true));

	// south-west
	cell.create(pos.x - 1, pos.y - 1);
	if (App->pathfinding->IsWalkable(cell))
		list_to_fill.pathNodeList.push_back(PathNode(-1, -1, cell, this, { 0,0 }, true));

	// north-west
	cell.create(pos.x + 1, pos.y + 1);
	if (App->pathfinding->IsWalkable(cell))
		list_to_fill.pathNodeList.push_back(PathNode(-1, -1, cell, this, { 0,0 }, true));

	// south-est
	cell.create(pos.x - 1, pos.y + 1);
	if (App->pathfinding->IsWalkable(cell))
		list_to_fill.pathNodeList.push_back(PathNode(-1, -1, cell, this, { 0,0 }, true));

	return list_to_fill.pathNodeList.size();
}


// PathNode -------------------------------------------------------------------------
// Calculates this tile score
// ----------------------------------------------------------------------------------
int PathNode::Score() const
{
	return g + h;
}

// PathNode -------------------------------------------------------------------------
// Calculate the F for a specific destination tile
// ----------------------------------------------------------------------------------
int PathNode::CalculateF(const iPoint& destination)
{
	if (isDiagonal)
			g = parent->g + 1.7f;
		else
			g = parent->g + 1.0f;

		h = pos.DistanceManhattan(destination);
	/*g = parent->g + 1;
	h = pos.DistanceTo(destination);*/

	return g + h;
}

// ----------------------------------------------------------------------------------
// Actual A* algorithm: return number of steps in the creation of the path or -1 ----
// ----------------------------------------------------------------------------------
int j1PathFinding::CreatePath(const iPoint& origin, const iPoint& destination)
{
	closed.pathNodeList.clear();
	open.pathNodeList.clear();
	int ret = 0;
	last_path.clear();

	// If origin or destination are not walkable leave
	if (!IsWalkable(origin) || !IsWalkable(destination))
	{
		return -1;
	}


	//PathList open;
	//PathList closed;

	PathNode originNode(0, origin.DistanceTo(destination), origin, nullptr);
	open.pathNodeList.push_back(originNode);

	while (open.pathNodeList.size() > 0) {

		// Move the lowest score cell from open list to the closed list
		PathNode* curr = (PathNode*)open.GetNodeLowestScore();
		closed.pathNodeList.push_back(*curr);

		// Erase element from list -----
		std::list<PathNode>::iterator it = open.pathNodeList.begin();
		while (it != open.pathNodeList.end()) {

			if (&(*it) == &(*curr))
				break;
			it++;
		}
		open.pathNodeList.erase(it);
		// Erase element from list -----

		// If we just added the destination, we are done!
		// Backtrack to create the final path
		if (closed.pathNodeList.back().pos == destination) {

			for (PathNode iterator = closed.pathNodeList.back(); iterator.parent != nullptr;
				iterator = *closed.Find(iterator.parent->pos)) {

				last_path.push_back(iterator.pos);
			}

			last_path.push_back(closed.pathNodeList.front().pos);

			// Flip() the path when you are finish
			std::reverse(last_path.begin(), last_path.end());

			ret = last_path.size();

			if (ret != -1)
			{
				for (uint i = 0; i < ret; ++i)
				{
					LOG("x = %i, y = %i", last_path[i].x, last_path[i].y);
				}
			}

			return ret;

		}
		else {
			// Fill a list of all adjancent nodes
			PathList neighbors;
			closed.pathNodeList.back().FindWalkableAdjacents(neighbors);

			// Iterate adjancent nodes:
			std::list<PathNode>::iterator iterator = neighbors.pathNodeList.begin();

			while (iterator != neighbors.pathNodeList.end()) {
				// ignore nodes in the closed list
				if (closed.Find((*iterator).pos) != NULL) {
					iterator++;
					continue;
				}

				(*iterator).CalculateF(destination);
				// If it is already in the open list, check if it is a better path (compare G)
				if (open.Find((*iterator).pos) != NULL) {

					// If it is a better path, Update the parent
					PathNode open_node = *open.Find((*iterator).pos);
					if ((*iterator).g < open_node.g)
						open_node.parent = (*iterator).parent;
				}
				else {
					// If it is NOT found, calculate its F and add it to the open list
					open.pathNodeList.push_back(*iterator);
				}
				iterator++;
			}
			neighbors.pathNodeList.clear();
		}
	}

	return ret;

}


PathState j1PathFinding::StartAStar(const iPoint & origin, const iPoint & destination)
{
	closed.pathNodeList.clear();
	open.pathNodeList.clear();
	last_path.clear();

	goal = destination;

	if (!IsWalkable(origin) || !IsWalkable(goal))
	{
		return PathState::Unavailable; 
	}
	PathNode originNode(0, origin.DistanceTo(goal), origin, nullptr);
	open.pathNodeList.push_back(originNode);

	return PathState::MAX; 
}


PathState j1PathFinding::CycleAStar()
{
	if (open.pathNodeList.empty() == false)
	{

		// Move the lowest score cell from open list to the closed list
		PathNode* curr = (PathNode*)open.GetNodeLowestScore();
		closed.pathNodeList.push_back(*curr);

		// Erase element from list -----
		std::list<PathNode>::iterator it = open.pathNodeList.begin();
		while (it != open.pathNodeList.end()) {

			if (&(*it) == &(*curr))
				break;
			it++;
		}
		open.pathNodeList.erase(it);
		// Erase element from list -----

		// If we just added the destination, we are done!
		// Backtrack to create the final path
		if (closed.pathNodeList.back().pos == goal) {

			for (PathNode iterator = closed.pathNodeList.back(); iterator.parent != nullptr;
				iterator = *closed.Find(iterator.parent->pos)) {

				last_path.push_back(iterator.pos);
			}

			last_path.push_back(closed.pathNodeList.front().pos);

			// Flip() the path when you are finish
			std::reverse(last_path.begin(), last_path.end());
			
			for (uint i = 0; i < last_path.size(); ++i)
			{
				LOG("x = %i, y = %i", last_path[i].x, last_path[i].y);
			}
		
			return PathState::Found;

		}
		else {
			// Fill a list of all adjancent nodes
			PathList neighbors;
			closed.pathNodeList.back().FindWalkableAdjacents(neighbors);

			// Iterate adjancent nodes:
			std::list<PathNode>::iterator iterator = neighbors.pathNodeList.begin();

			while (iterator != neighbors.pathNodeList.end()) {
				// ignore nodes in the closed list
				if (closed.Find((*iterator).pos) != NULL) {
					iterator++;
					continue;
				}

				(*iterator).CalculateF(goal);
				// If it is already in the open list, check if it is a better path (compare G)
				if (open.Find((*iterator).pos) != NULL) {

					// If it is a better path, Update the parent
					PathNode open_node = *open.Find((*iterator).pos);
					if ((*iterator).g < open_node.g)
						open_node.parent = (*iterator).parent;
				}
				else {
					// If it is NOT found, calculate its F and add it to the open list
					open.pathNodeList.push_back(*iterator);
				}
				iterator++;
			}
			neighbors.pathNodeList.clear();
		}
	}

	return PathState::Searching;
}

PathState j1PathFinding::StartJPS(const iPoint & origin, const iPoint & destination)
{
	closed.pathNodeList.clear();
	open.pathNodeList.clear();
	last_path.clear();

	goal = destination;

	if (!IsWalkable(origin) || !IsWalkable(goal))
	{
		return PathState::Unavailable;
	}


	// HORIZONTAL CASES 

	// East
	//PathNode originNode();
	open.pathNodeList.push_back(PathNode(0, origin.DistanceManhattan(goal), origin, nullptr, { 1,0 }));

	// West
	//PathNode originNode(0, origin.DistanceManhattan(goal), origin, nullptr, iPoint(-1, 0));
	open.pathNodeList.push_back(PathNode(0, origin.DistanceManhattan(goal), origin, nullptr, { -1,0 }));


	// VERTICAL CASES 

	// North
	//PathNode originNode(0, origin.DistanceManhattan(goal), origin, nullptr, iPoint(0, 1));
	open.pathNodeList.push_back(PathNode(0, origin.DistanceManhattan(goal), origin, nullptr, { 0, 1 }));

	// South
	//PathNode originNode(0, origin.DistanceManhattan(goal), origin, nullptr, iPoint(0, -1));
	open.pathNodeList.push_back(PathNode(0, origin.DistanceManhattan(goal), origin, nullptr, { 0, -1 }));
	

	// DIAGONAL CASES 

	// North - East
	//PathNode originNode(0, origin.DistanceManhattan(goal), origin, nullptr, iPoint(1, 1));
	open.pathNodeList.push_back(PathNode(0, origin.DistanceManhattan(goal), origin, nullptr, { 1, 1 }));

	// South - East
	//PathNode originNode(0, origin.DistanceManhattan(goal), origin, nullptr, iPoint(1, -1));
	open.pathNodeList.push_back(PathNode(0, origin.DistanceManhattan(goal), origin, nullptr, { 1, -1 }));

	// South - West
	//PathNode originNode(0, origin.DistanceManhattan(goal), origin, nullptr, iPoint(-1, -1));
	open.pathNodeList.push_back(PathNode(0, origin.DistanceManhattan(goal), origin, nullptr, { -1, -1 }));

	// North - West
	//PathNode originNode(0, origin.DistanceManhattan(goal), origin, nullptr, iPoint(-1, 1));
	open.pathNodeList.push_back(PathNode(0, origin.DistanceManhattan(goal), origin, nullptr, { -1, 1 }));


	return PathState::MAX;
}

PathState j1PathFinding::CycleJPS()
{
	// Not pointer
	/*PathNode currNode = open.pathNodeList.back(); 
	open.pathNodeList.pop_back();

	if (currNode.direction.y == 0)
		HorizontalJump(currNode);
	else if (currNode.direction.x == 0)
		VerticalJump(currNode); 
	else DiagonalJump(currNode);*/

	// Not pointer
	PathNode currNode = open.pathNodeList.back();
	open.pathNodeList.pop_back();

	if (currNode.direction.y == 0)
		HorizontalJumpPtr(currNode.pos, currNode.direction, &currNode);
	else if (currNode.direction.x == 0)
		VerticalJumpPtr(currNode.pos, currNode.direction, &currNode);
	else DiagonalJumpPtr(currNode.pos, currNode.direction, &currNode);


	return PathState::MAX;
}

bool j1PathFinding::HorizontalJump(const PathNode& node)
{
	bool gotJumpPoint = false;

	iPoint newPos = node.pos + node.direction;
	int horizontalDir = node.direction.x; 

	if (!IsWalkable(newPos))
	{
		return false; 
	}

	if (newPos == goal)
	{
		open.pathNodeList.push_back(PathNode(-1, -1, newPos, &node));
		return true;
	}


	if (!IsWalkable(newPos + iPoint(0, 1)) && IsWalkable(newPos + iPoint(horizontalDir, 1)))
	{
		open.pathNodeList.push_back(PathNode(-1, -1, newPos, &node, { horizontalDir, 1 }));
		gotJumpPoint = true; 
	}
	
	if (!IsWalkable(newPos + iPoint(0, -1)) && IsWalkable(newPos + iPoint(horizontalDir, -1)))
	{
		open.pathNodeList.push_back(PathNode(-1, -1, newPos, &node, { horizontalDir, -1 }));
		gotJumpPoint = true;
	}

	closed.pathNodeList.push_back(PathNode(-1, -1, newPos, &node));

	if (gotJumpPoint)
	{
		open.pathNodeList.push_back(PathNode(-1, -1, newPos, &node, node.direction));
		return true; 
	}
	else 	HorizontalJump(PathNode(-1, -1, newPos, &node, node.direction));
}

bool j1PathFinding::VerticalJump(const PathNode & node)
{
	bool gotJumpPoint = false;

	iPoint newPos = node.pos + node.direction;
	int verticalDir = node.direction.y; 

	if (!IsWalkable(newPos))
	{
		return false;
	}

	if (newPos == goal)
	{
		open.pathNodeList.push_back(PathNode(-1, -1, newPos, &node));
		return true;
	}

	if (!IsWalkable(newPos + iPoint(1, 0)) && IsWalkable(newPos + iPoint(1, verticalDir)))
	{
		open.pathNodeList.push_back(PathNode(-1, -1, newPos, &node, { 1, verticalDir }));
		gotJumpPoint = true;
	}

	if (!IsWalkable(newPos + iPoint(-1, 0)) && IsWalkable(newPos + iPoint(-1, verticalDir)))
	{
		open.pathNodeList.push_back(PathNode(-1, -1, newPos, &node, { -1, verticalDir }));
		gotJumpPoint = true;
	}

	closed.pathNodeList.push_back(PathNode(-1, -1, newPos, &node));

	if (gotJumpPoint)
	{
		open.pathNodeList.push_back(PathNode(-1, -1, newPos, &node, node.direction));
		return true;
	}
	else VerticalJump(PathNode(-1, -1, newPos, &node, node.direction));

}

bool j1PathFinding::DiagonalJump(const PathNode & node)
{
	bool gotJumpPoint = false; 

	iPoint oldPos = node.pos; 
	iPoint newPos = node.pos + node.direction; 
	
	int horizontalDir = node.direction.x; 
	int verticalDir = node.direction.y; 

	if (!IsWalkable(newPos))
		return false;
	
	if (newPos == goal) 
	{
		open.pathNodeList.push_back(PathNode(-1, -1, newPos, &node));
		return false; 
	}
	
	if (!IsWalkable(newPos + iPoint(-horizontalDir, 0)) && IsWalkable(newPos + iPoint(-horizontalDir, 1)))
	{
		open.pathNodeList.push_back(PathNode(-1, -1, newPos, &node, { -horizontalDir, 1 }));

		// Extra cases? 
		open.pathNodeList.push_back(PathNode(-1, -1, newPos, &node, { horizontalDir, 0 }));
		open.pathNodeList.push_back(PathNode(-1, -1, newPos, &node, { 0, verticalDir }));
		gotJumpPoint = true;
	} 

	if (!IsWalkable(newPos + iPoint(0, -verticalDir)) && IsWalkable(newPos + iPoint(1, -verticalDir)))
	{
		open.pathNodeList.push_back(PathNode(-1, -1, newPos, &node, { 1, -verticalDir }));

		// Extra cases? 
		open.pathNodeList.push_back(PathNode(-1, -1, newPos, &node, { horizontalDir, 0 }));
		open.pathNodeList.push_back(PathNode(-1, -1, newPos, &node, { 0, verticalDir }));
		gotJumpPoint = true;
	}

	if (gotJumpPoint)
	{
		open.pathNodeList.push_back(PathNode(-1, -1, newPos, &node, node.direction));
		return true;
	}

	if (HorizontalJump(PathNode(-1, -1, node.pos, &node, { horizontalDir, 0 })) != true)
	{
		if (VerticalJump(PathNode(-1, -1, node.pos, &node, { 0, verticalDir })) != true)
		{
			closed.pathNodeList.push_back(PathNode(-1, -1, newPos, &node));
			return DiagonalJump(PathNode(-1, -1, newPos, &node, node.direction));
		}
	}
}

PathNode* j1PathFinding::HorizontalJumpPtr(const iPoint & position, const iPoint & direction, const PathNode * parent)
{
	bool gotJumpPoint = false; 

	iPoint newPos = position + direction;

	int horizontalDir = direction.x;

	/*PathList nodes;
	PathNode currNode(-1, -1, newPos, parent, direction);*/

	PathNode* currNode = new PathNode(-1, -1, newPos, parent, direction); 

	if (!IsWalkable(newPos))
	{
		return nullptr;
	}

	if (newPos == goal)
	{
		//open.pathNodeList.push_back(PathNode(-1, -1, newPos, parent));
		return currNode;
	}


	if (!IsWalkable(newPos + iPoint(0, 1)) && IsWalkable(newPos + iPoint(horizontalDir, 1)))
	{
		open.pathNodeList.push_back(PathNode(-1, -1, newPos, parent, { horizontalDir, 1 }));
		gotJumpPoint = true; 
	}

	if (!IsWalkable(newPos + iPoint(0, -1)) && IsWalkable(newPos + iPoint(horizontalDir, -1)))
	{
		open.pathNodeList.push_back(PathNode(-1, -1, newPos, parent, { horizontalDir, -1 }));
		gotJumpPoint = true; 
	}

	closed.pathNodeList.push_back(PathNode(-1, -1, newPos, parent));

	if (gotJumpPoint)
	{
		//nodes.pathNodeList.push_back(PathNode(-1, -1, newPos, parent, direction));
		return currNode;
	}
	else return HorizontalJumpPtr(newPos, direction, parent);

}

PathNode* j1PathFinding::VerticalJumpPtr(const iPoint & position, const iPoint & direction, const PathNode * parent)
{
	bool gotJumpPoint = false; 

	iPoint newPos = position + direction;

	int verticalDir = direction.x;

	/*PathList nodes;
	PathNode currNode(-1, -1, newPos, parent, direction);*/

	PathNode* currNode = new PathNode(-1, -1, newPos, parent, direction);

	if (!IsWalkable(newPos))
	{
		return nullptr;
	}

	if (newPos == goal)
	{
		//open.pathNodeList.push_back(PathNode(-1, -1, newPos, parent));
		return currNode;
	}


	if (!IsWalkable(newPos + iPoint(1, 0)) && IsWalkable(newPos + iPoint(1, verticalDir)))
	{
		open.pathNodeList.push_back(PathNode(-1, -1, newPos, parent, { 1, verticalDir }));
		gotJumpPoint = true; 
	}

	if (!IsWalkable(newPos + iPoint(-1, 0)) && IsWalkable(newPos + iPoint(-1, verticalDir)))
	{
		open.pathNodeList.push_back(PathNode(-1, -1, newPos, parent, { -1, verticalDir }));
		gotJumpPoint = true; 
	}

	closed.pathNodeList.push_back(PathNode(-1, -1, newPos, parent));

	if (gotJumpPoint)
	{
		//nodes.pathNodeList.push_back(PathNode(-1, -1, newPos, parent, direction));
		return currNode;
	}
	else return VerticalJumpPtr(newPos, direction, parent);
}

PathNode* j1PathFinding::DiagonalJumpPtr(const iPoint & position, const iPoint & direction, const PathNode * parent)
{
	bool gotJumpPoint = false;

	iPoint newPos = position + direction;

	/*bool horDone = false; 
	bool verDone = false; */

	int horizontalDir = direction.x;
	int verticalDir = direction.y;

	/*PathList nodes;
	PathNode currNode(-1, -1, newPos, parent, direction);*/

	PathNode* currNode = new PathNode(-1, -1, newPos, parent, direction);

	if (!IsWalkable(newPos))
		return nullptr;

	if (newPos == goal)
	{
		//nodes.pathNodeList.push_back(PathNode(-1, -1, newPos, parent));
		return currNode;
	}

	if (!IsWalkable(newPos + iPoint(-horizontalDir, 0)) && IsWalkable(newPos + iPoint(-horizontalDir, 1)))
	{
		open.pathNodeList.push_back(PathNode(-1, -1, newPos, parent, { -horizontalDir, 1 }));

		// Extra cases? 
		open.pathNodeList.push_back(PathNode(-1, -1, newPos, parent, { horizontalDir, 0 }));
		open.pathNodeList.push_back(PathNode(-1, -1, newPos, parent, { 0, verticalDir }));
		gotJumpPoint = true;
	}

	if (!IsWalkable(newPos + iPoint(0, -verticalDir)) && IsWalkable(newPos + iPoint(1, -verticalDir)))
	{
		open.pathNodeList.push_back(PathNode(-1, -1, newPos, parent, { 1, -verticalDir }));

		// Extra cases? 
		open.pathNodeList.push_back(PathNode(-1, -1, newPos, parent, { horizontalDir, 0 }));
		open.pathNodeList.push_back(PathNode(-1, -1, newPos, parent, { 0, verticalDir }));

		gotJumpPoint = true;
	}

	if (HorizontalJumpPtr(position, { horizontalDir, 0 }, parent) != nullptr)
	{
		return HorizontalJumpPtr(position, { horizontalDir, 0 }, parent);
	}
	else if (VerticalJumpPtr(position, { 0, verticalDir }, parent) != nullptr)
	{
		return HorizontalJumpPtr(position, { horizontalDir, 0 }, parent);
	}

	closed.pathNodeList.push_back(PathNode(-1, -1, newPos, parent));

	if (gotJumpPoint)
	{
		//nodes.pathNodeList.push_back(PathNode(-1, -1, newPos, parent, direction));
		return currNode;
	}
	else return DiagonalJumpPtr(newPos, direction, parent); 

	//PathList* subNodes = nullptr; 

	//if (nodes.pathNodeList.empty() == true)
	//{
	//	subNodes = HorizontalJumpPtr(position, { horizontalDir, 0 }, parent);
	//	horDone = true; 
	//}

	//if (subNodes != nullptr)
	//{
	//	std::list<PathNode>::iterator iter = subNodes->pathNodeList.begin(); 
	//	for (iter; iter != subNodes->pathNodeList.end(); iter++)
	//	{
	//		nodes.pathNodeList.push_back((*iter)); 
	//	}
	//}
	//subNodes = nullptr; 

	//if (nodes.pathNodeList.empty() == true)
	//{
	//	subNodes = VerticalJumpPtr(position, { horizontalDir, 0 }, parent);
	//	verDone = true;
	//}

	//if (subNodes != nullptr)
	//{
	//	std::list<PathNode>::iterator iter = subNodes->pathNodeList.begin();
	//	for (iter; iter != subNodes->pathNodeList.end(); iter++)
	//	{
	//		nodes.pathNodeList.push_back((*iter));
	//	}
	//}


	//if (nodes.pathNodeList.empty() == true)
	//{
	//	subNodes = DiagonalJumpPtr(newPos, direction, parent); 
	//}
	
	//if (HorizontalJumpPtr(position, { horizontalDir, 0 }, parent)->pathNodeList.empty() == true)
	//{
	//	if (VerticalJumpPtr(position, { 0, verticalDir }, parent)->pathNodeList.empty() == true)
	//	{
	//		closed.pathNodeList.push_back(PathNode(-1, -1, newPos, parent));
	//		return DiagonalJumpPtr(newPos, direction, parent);
	//	}
	//}
}

PathList * j1PathFinding::UltraJump(const iPoint & position, const iPoint & direction, const PathNode * parent)
{
	return nullptr;
}

bool j1PathFinding::CheckForcedNeighboursHor(const iPoint & pos, const iPoint & direction)
{
	bool ret = false; 

	if (!IsWalkable(pos + iPoint(0, 1)) && IsWalkable(pos + iPoint(direction.x, 1)))
	{
		ret = true; 
	}

	if (!IsWalkable(pos + iPoint(0, -1)) && IsWalkable(pos + iPoint(direction.x, -1)))
	{
		ret = true; 
	}

	return ret;
}

bool j1PathFinding::CheckForcedNeighboursVer(const iPoint & pos, const iPoint & direction)
{
	bool ret = false;

	if (!IsWalkable(pos + iPoint(1, 0)) && IsWalkable(pos + iPoint(1, direction.y)))
	{
		ret = true;
	}

	if (!IsWalkable(pos + iPoint(-1, 0)) &&IsWalkable(pos + iPoint(-1, direction.y)))
	{
		ret = true;
	}

	return ret;
}

bool j1PathFinding::CheckForcedNeighboursDiag(const iPoint & pos, const iPoint & direction)
{
	bool ret = false; 

	if (!IsWalkable(pos + iPoint(-direction.x, 0)) && IsWalkable(pos + iPoint(-direction.x, 1)))
	{
		ret = true;
	}

	if (!IsWalkable(pos + iPoint(0, -direction.y)) && IsWalkable(pos + iPoint(1, -direction.y)))
	{
		ret = true;
	}

	return ret;
}
	