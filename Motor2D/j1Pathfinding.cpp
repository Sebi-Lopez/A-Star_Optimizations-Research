#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1PathFinding.h"
#include "j1Render.h"
#include "j1Map.h"
#include "j1Textures.h"
#include "j1Timer.h"

j1PathFinding::j1PathFinding() : j1Module(), map(NULL), last_path(0), width(0), height(0)
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
	return true;
}

bool j1PathFinding::PostUpdate()
{
	DebugDraw(); 

	return true;
}

void j1PathFinding::DebugDraw()
{
	iPoint pos = { 0,0 };

	// Draw Open Queue
	
	
	std::list<PathNode>::iterator iterator = visited.pathNodeList.begin(); 

	// Draw Visited Queue
	for (iterator; iterator != visited.pathNodeList.end(); iterator++)
	{
		pos = App->map->MapToWorld((*iterator).pos.x, (*iterator).pos.y);
		App->render->DrawQuad({ pos.x,pos.y,App->map->data.tile_width, App->map->data.tile_height }, 150, 50, 255, 100);
	}

	iterator = open.pathNodeList.begin();
	for (iterator; iterator != open.pathNodeList.end(); iterator++)
	{
		pos = App->map->MapToWorld((*iterator).pos.x, (*iterator).pos.y);
		App->render->DrawQuad({ pos.x,pos.y,App->map->data.tile_width, App->map->data.tile_height }, 0, 255, 0, 255);
	}
	// Draw Closed Queue
	iterator = closed.pathNodeList.begin();
	for (iterator; iterator != closed.pathNodeList.end(); iterator++)
	{
		pos = App->map->MapToWorld((*iterator).pos.x, (*iterator).pos.y);
		App->render->DrawQuad({ pos.x,pos.y,App->map->data.tile_width, App->map->data.tile_height }, 255, 0, 0, 255);
	}

	// Draw Start Point
	pos = App->map->MapToWorld(origin.x, origin.y);
	App->render->DrawQuad({ pos.x,pos.y,App->map->data.tile_width, App->map->data.tile_height }, 0, 0, 255, 255);

	// Draw Goal
	pos = App->map->MapToWorld(goal.x, goal.y);
	App->render->DrawQuad({ pos.x,pos.y,App->map->data.tile_width, App->map->data.tile_height }, 255, 0, 0, 255);

	DrawGrid(); 
}

void j1PathFinding::DrawGrid()
{
	j1Timer timer;
	timer.Start();

	// Horizontal	
	for (int x = 0; x < App->map->data.height + 1; ++x)
	{
		iPoint startPoint = App->map->MapToWorld(0, x);
		iPoint finalPoint = App->map->MapToWorld(App->map->data.width, x);
		App->render->DrawLine(startPoint.x, startPoint.y, finalPoint.x, finalPoint.y, 0, 0, 0);
	}

	// Vertical
	for (int j = 1; j < App->map->data.width + 1; ++j)
	{
		iPoint startPoint = App->map->MapToWorld(j, 0);
		iPoint finalPoint = App->map->MapToWorld(j, App->map->data.height);
		App->render->DrawLine(startPoint.x, startPoint.y, finalPoint.x, finalPoint.y, 0, 0, 0);
	}
	timer.Read();
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
	if (CheckBoundaries(pos))
		return map[(pos.y*width) + pos.x];

	return INVALID_WALK_CODE;
}

uint j1PathFinding::GetIndexAt(const iPoint & pos) const
{
	return uint(pos.y * width + pos.x);
}

iPoint j1PathFinding::GetTileFromWalkability(int id) const
{
	return iPoint((id % width), (int) (id / width));
}

iPoint j1PathFinding::GetPosFromWalkability(int id) const
{
	int x = (id % width);
	int y = (int)(id / width);

	iPoint ret = App->map->MapToWorld(x, y);
	return ret;
}

void j1PathFinding::SwapValue(int id)
{
	if (id < width * height)
	{
		if (map[id] == 0)
			map[id] = 1; 
		else map[id] = 0; 
	}
}

void j1PathFinding::ActivateTile(const iPoint& tile)
{
	if (CheckBoundaries(tile))
	{
		int id = GetIndexAt(tile);
		if (map[id] != 0)
			map[id] = 0; 
	}
}

void j1PathFinding::DeactivateTile(const iPoint& tile)
{
	if (CheckBoundaries(tile))
	{
		int id = GetIndexAt(tile); 
		if (map[id] == 0)
			map[id] = 1; 		
	}
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
PathNode::PathNode() : g(-1), h(-1), pos(-1, -1), parent(nullptr)
{}

PathNode::PathNode(int g, int h, const iPoint& pos, const PathNode* parent, bool isDiagonal) : g(g), h(h), pos(pos), parent(parent), isDiagonal(isDiagonal)
{}

PathNode::PathNode(const PathNode& node) : g(node.g), h(node.h), pos(node.pos), parent(node.parent)
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
		list_to_fill.pathNodeList.push_back(PathNode(-1, -1, cell, this, true));

	// south-west
	cell.create(pos.x - 1, pos.y - 1);
	if (App->pathfinding->IsWalkable(cell))
		list_to_fill.pathNodeList.push_back(PathNode(-1, -1, cell, this, true));

	// north-west
	cell.create(pos.x + 1, pos.y + 1);
	if (App->pathfinding->IsWalkable(cell))
		list_to_fill.pathNodeList.push_back(PathNode(-1, -1, cell, this, true));

	// south-est
	cell.create(pos.x - 1, pos.y + 1);
	if (App->pathfinding->IsWalkable(cell))
		list_to_fill.pathNodeList.push_back(PathNode(-1, -1, cell, this, true));

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
	
	return g + h;
}

int PathNode::CalculateFJPS(const iPoint & destination)
{
	g = parent->g + parent->pos.DistanceManhattan(destination);

	h = pos.DistanceManhattan(destination);

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

int j1PathFinding::CreatePathJPS(const iPoint & origin, const iPoint & destination)
{
	closed.pathNodeList.clear();
	open.pathNodeList.clear();
	visited.pathNodeList.clear();
	last_path.clear();

	goal = destination;
	this->origin = origin;

	if (!IsWalkable(origin) || !IsWalkable(goal))
	{
		return -1;
	}
	open.pathNodeList.push_back(PathNode(0, origin.DistanceManhattan(goal), origin, nullptr));

	while (open.pathNodeList.empty() == false)
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
		if ((*curr).pos == goal) {

			for (PathNode iterator = (*curr); iterator.parent != nullptr;
				iterator = *closed.Find(iterator.parent->pos)) {

				last_path.push_back(iterator.pos);
			}

			last_path.push_back(closed.pathNodeList.front().pos);

			/*closed.pathNodeList.clear();
			open.pathNodeList.clear();
			visited.pathNodeList.clear();*/
			// Flip() the path when you are finish
			std::reverse(last_path.begin(), last_path.end());

			for (uint i = 0; i < last_path.size(); ++i)
			{
				LOG("x = %i, y = %i", last_path[i].x, last_path[i].y);
			}


			return last_path.size();

		}
		else {
			// Fill a list of all adjancent nodes
			PathList neighbors;
			closed.pathNodeList.back().PruneNeighbours(neighbors);

			// Iterate adjancent nodes:
			std::list<PathNode>::iterator iterator = neighbors.pathNodeList.begin();

			while (iterator != neighbors.pathNodeList.end()) {
				// ignore nodes in the closed list
				if (closed.Find((*iterator).pos) != nullptr) {
					iterator++;
					continue;
				}

				(*iterator).CalculateFJPS(goal);
				// If it is already in the open list, check if it is a better path (compare G)
				if (open.Find((*iterator).pos) != nullptr) {

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
	return -1;
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
		if ((*curr).pos == goal) {

			for (PathNode iterator = (*curr); iterator.parent != nullptr;
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
				if (closed.Find((*iterator).pos) != nullptr) {
					iterator++;
					continue;
				}

				(*iterator).CalculateF(goal);
				// If it is already in the open list, check if it is a better path (compare G)
				if (open.Find((*iterator).pos) != nullptr) {

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
	visited.pathNodeList.clear();
	last_path.clear();

	goal = destination;
	this->origin = origin; 

	if (!IsWalkable(origin) || !IsWalkable(goal))
	{
		return PathState::Unavailable;
	}
	open.pathNodeList.push_back(PathNode(0, origin.DistanceManhattan(goal), origin, nullptr));


	return PathState::MAX;
}

PathState j1PathFinding::CycleJPS()
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
		if ((*curr).pos == goal) {

			for (PathNode iterator = (*curr); iterator.parent != nullptr;
				iterator = *closed.Find(iterator.parent->pos)) {

				last_path.push_back(iterator.pos);
			}

			last_path.push_back(closed.pathNodeList.front().pos);

			closed.pathNodeList.clear(); 
			open.pathNodeList.clear(); 
			visited.pathNodeList.clear();
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
			closed.pathNodeList.back().PruneNeighbours(neighbors); 

			// Iterate adjancent nodes:
			std::list<PathNode>::iterator iterator = neighbors.pathNodeList.begin();

			while (iterator != neighbors.pathNodeList.end()) {
				// ignore nodes in the closed list
				if (closed.Find((*iterator).pos) != nullptr) {
					iterator++;
					continue;
				}

				(*iterator).CalculateFJPS(goal);
				// If it is already in the open list, check if it is a better path (compare G)
				if (open.Find((*iterator).pos) != nullptr) {

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
	return PathState::MAX;
}

PathNode j1PathFinding::Jump(const iPoint & pos, const iPoint & direction, const PathNode * parent)
{
	iPoint newPos = pos + direction; 

	PathNode newNode(-1, -1, newPos, parent); 

	if (!IsWalkable(newPos))
		return PathNode(-1, -1, iPoint(-1, -1), nullptr); 

	if (newPos == goal)
		return newNode; 

	visited.pathNodeList.push_back(PathNode(-1, -1, newPos, nullptr));

	if (direction.x != 0 && direction.y != 0)
	{
		if (!IsWalkable(pos + iPoint(direction.x, 0)) && IsWalkable(pos + iPoint(direction.x + direction.x, 0)))
			return newNode; 
		if (!IsWalkable(pos + iPoint(0, direction.y)) && IsWalkable(pos + iPoint (0, direction.y + direction.y)))
			return newNode; 

		if (Jump(newPos, iPoint(direction.x, 0), parent).pos != iPoint(-1, -1)
			|| Jump(newPos, iPoint(0, direction.y), parent).pos != iPoint(-1, -1))
			return newNode;
		
	}
	else
	{
		if (direction.y != 0)

		{
			if (!IsWalkable(newPos + iPoint(1, 0)) && IsWalkable(newPos + iPoint(1, direction.y)))
				return newNode;

			if (!IsWalkable(newPos + iPoint(-1, 0)) && IsWalkable(newPos + iPoint(-1, direction.y)))
				return newNode;
		}

		else
		{
			if (!IsWalkable(newPos + iPoint(0, 1)) && IsWalkable(newPos + iPoint(direction.x, 1)))
				return newNode;

			if (!IsWalkable(newPos + iPoint(0, -1)) && IsWalkable(newPos + iPoint(direction.x, -1)))
				return newNode;
		}
	}

	return Jump(newPos, direction, parent);

}


void PathNode::PruneNeighbours(PathList & list_to_fill) 
{
	PathList neighbours; 
	FindWalkableAdjacents(neighbours); 

	std::list<PathNode>::iterator iter = neighbours.pathNodeList.begin(); 

	for (iter; iter != neighbours.pathNodeList.end(); iter++)
	{
		
		iPoint direction(CLAMP((*iter).pos.x - pos.x, 1, -1), CLAMP((*iter).pos.y - pos.y, 1, -1));

		PathNode returnedJP = App->pathfinding->Jump(pos, direction, this);
		if (returnedJP.pos != iPoint(-1, -1))
			list_to_fill.pathNodeList.push_back(returnedJP); 
	}
}
