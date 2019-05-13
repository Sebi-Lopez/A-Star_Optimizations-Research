# Introduction
I am [Sebastià López Tenorio](https://github.com/Sebi-Lopez), student of the [Bachelor’s Degree in Video Games by UPC at CITM](https://www.citm.upc.edu/ing/estudis/graus-videojocs/). This content is generated for the second year’s subject Project 2, under the supervision of lecturer [Ricard Pillosu](https://es.linkedin.com/in/ricardpillosu)

You can download the repository [here](https://github.com/Sebi-Lopez/A-Star_Optimizations-Research). 
# Current Position

In videogames, the use of a pathfinding algorithm is crucial when dealing with all sorts of movements. 
The algorithm that we are using now, the A*, as many of you know, can be really slow. Most of all when we have to deal with a larger map. At some point, the number of nodes that this algorithm has to explore and look through make it unusable. Either it takes too long to give a path (making unities look unresponsive) or the FPS of the game drop (disaster). This problem grows as well with the number of units demanding path at the same time. 

It's for this reason we are going to find a way we can improve this algorithm. 

# Possible Optimizations
There are lots of possibilities to optimize the A* each one with its pros and cons. I'm not going to go through all of them, but I'm going to try to briefly explain the main mechanics of some that I found interesting to take a look at. 


## Potential Fields

[Potential fields](http://aigamedev.com/open/tutorials/potential-fields/) are a goal based pathfinding algorythm that consists in finding "every possible" path to the goal. As the name suggests, the focus is on the goal position, and expand the pathfinding algorithm from it. Eventually, it will found the shortest path to the goal for every node on the map. In every node, there will be an orientation, that indicates the direction following this optimal path. The unit that wants to go to the goal, will only have to refer to that and, and follow its vector, to get closer to the goal.


<p align="center">
<img src="https://github.com/Sebi-Lopez/A-Star_Optimizations-Research/blob/master/docs/images/VectorFields/vectorField2.png?raw=true" width="200">
</p>


This [article](https://gamedevelopment.tutsplus.com/tutorials/understanding-goal-based-vector-field-pathfinding--gamedev-9007) explains it, with a explanation [video](https://www.youtube.com/watch?v=Bspb9g9nTto). 

This method is really effective when we want a larger number of entities demanding path to that goal. The author of the article 
However, this algorythm is not very efficient when we have a larger map, as calculating the vector field can get really expensive because the number of nodes. Also, there needs to be another algorythm that needs to work 

## Lifelong Planning A*


## Theta*


## ARA* (Anytime Repairing A*)


## HPA* 
This algorithm has taken over the game industry. Is one of the most used in the present. 
Hierarchical Path-Finding A* is a pathfinding method that abstracts any grid-map to different sets of "clusters" or "blocks" with different levels of abstraction. In their [paper](https://webdocs.cs.ualberta.ca/~mmueller/ps/hpastar.pdf) the creators use a metaphor with a car trip starting in one city and ending in a city in another country (both points with its respective addresses). Us, humans, can abstract the path we are going to take really well. We first look to get into a highway, then move from one state to another, and when we get to the destination city or state, we search at the "city level", within its streets and roundabouts. This method follows this abstraction principle. 

As mentioned, each cluster has information about its entries, its distances and costs. So travelling at "city level" can get very efficient. These clusters are made a clustering algorithm that groups neighbours together when appropriate. Therefore, there's no need for, let's say, the designer of the map, to add extra data when creating the map, as this algorithm makes the abstraction zones by itself. Consequently, this method doesn't have any problem to be added to a procedurally generated map.


## IDA* 

## Swamps

The usage of Swamps is another method that tries to avoid areas that are navigated unnecessary by heuristic methods like A*. It calculates zones this undesirable zones in pre-runtime. This way, avoid the expansion of the nodes in zones (that can get to be really big) in which we know the path won't pass. In fact, the path will only pass those zones if the end of the beginning of it is located in these zones. The picture below, extracted from the original [paper](http://leibniz.cs.huji.ac.il/tr/1188.pdf) shows the idea. 

<p align="center">
<img src="https://github.com/Sebi-Lopez/A-Star_Optimizations-Research/blob/master/docs/images/Swamps/swampExample.PNG?raw=true" width="200">
</p>


# Selected Approach: JPS
The  [Jump Point Search](http://users.cecs.anu.edu.au/~dharabor/data/papers/harabor-grastien-aaai11.pdf) is an algorithm build upon A* pathfinding algorithm and works in uniform-cost grid maps. It requires no preprocessing nor occupies memory (unlike most of the other optimizations) and it's compatible with other improving technics like abstraction.

I'm going to explain how it works, and afterwards, I'm going to explain the way I implemented it, guided by this [website](https://www.gamedev.net/articles/programming/artificial-intelligence/jump-point-search-fast-a-pathfinding-for-uniform-cost-grids-r4220/) that approaches the method in a different way. I made comparisons with both approaches and concluded that this one has better results. 

[Here](https://gamedevelopment.tutsplus.com/tutorials/how-to-speed-up-a-pathfinding-with-the-jump-point-search-algorithm--gamedev-5818)  there is a web that has been really helpful to build this algorythm, so feel free to take this as support as well. 

## Description
Its main purpose is to reduce the number of nodes in the open list of the A*. This optimizes the search speed for two reasons. 
1 - It reduces the number of operations needed to make a path.
2 - (Most Important) With fewer nodes in the open list every iteration to find the node with lower cost is cheaper. 

How does it do it? Well, the concept that you have to stick to your head is Path Symmetry.  

### Path Symmetry
His creator, Daniel Harabor, has made an incredible optimization of the A* by exploiting the path symmetry. The idea is that the A* algorithm looks through lots of similar paths that are symmetric, most of all in larger and open spaces. As you can see in the picture below, all those paths are equivalent when we talk about efficiency.  The only difference between one and another is which direction you take first. At the end of the day, you will do the same movements, in a different order.

<p align="center">
<img src="https://github.com/Sebi-Lopez/A-Star_Optimizations-Research/blob/master/docs/images/JPS/FPSB_symmetry.png?raw=true" width="340">
</p>

When talking about symmetric paths, the A* algorithm is forced to explore every node adjacent to the optimal path. In the past picture, depending on how we handle the situation of having two nodes with the same score, A* might even explore the whole map before reaching the destination. 


## Jumping 
The principal idea is that there is no need to explore every possible path (since most of them are symmetric), so not every node is interesting to look at. The algorithm "jumps over" these non-interesting nodes, avoiding to analyze them explicitly (adding them to the open list). It does this following two jump rules, also called Pruning Rules. 
Another way to look at what it does is saying that each jump, tries to "prove" that exists another path to the goal that is equally optimal (symmetric) and doesn't pass through certain nodes. Is a bit twisted, but you'll see it clearer now. 

## Pruning Rules
There are two main rules for pruning. These two are separated only by the direction of the jump we are trying to make. We differentiate between straight jumps (horizontal and vertical) and diagonal jumps. This [website](https://zerowidth.com/2013/05/05/jump-point-search-explained.html) helped me understand this concept, and I inspired this segment's explanation in it.

To discard the major number of nodes that we are not interested, we are going to look at it through the perspective, am I (as the node) really needed to be on the final path. To prove so, we are going to make sure, that there is no other "interesting" point that needs to be analyzed that forces the path to go through me (because I am part of the optimal path to get to that node). We will talk about this interesting nodes after explaining the jumps. 

### Horizontal Jumps: 
So I (again as a node) am trying to jump over the right direction (as my parent is on my left). 

<p align="center">
<img src="https://github.com/Sebi-Lopez/A-Star_Optimizations-Research/blob/master/docs/images/JPS/Prunning%20Examples/pruningHorizontalExample01.PNG?raw=true" width="200">
</p>

I can assume these two nodes, above and below of my parent, don't need to go through me to get there (they can easily go through my parent). 

<p align="center">
<img src="https://github.com/Sebi-Lopez/A-Star_Optimizations-Research/blob/master/docs/images/JPS/Prunning%20Examples/pruningHorizontalExample02.PNG?raw=true" width="200">
</p>

As well, I can assume that the nodes avobe and below me, are optimally obtained going through my parent diagonal (as its moving distance  is √2 and going through me the distance would be 2). 

<p align="center">
<img src="https://github.com/Sebi-Lopez/A-Star_Optimizations-Research/blob/master/docs/images/JPS/Prunning%20Examples/pruningHorizontalExample03.PNG?raw=true" width="200">
</p>

To get to the nodes diagonally more to the right of me, the optimal path can go through me, but as well, it can go through the two nodes that we just discarded, as the path is symmetric. We are going to assume, that that is the correct path to go (you will see why later).

<p align="center">
<img src="https://github.com/Sebi-Lopez/A-Star_Optimizations-Research/blob/master/docs/images/JPS/Prunning%20Examples/pruningHorizontalExample04.PNG?raw=true" width="250">
</p>

So now, we only have one direction to go, as all the others will be explored and analyzed by other jumps. So I will keep jumping nodes horizontally to the right, until I encounter with a wall. Then my jump will be over and I can guarantee that there are no interesting points that need to pass through that row of nodes, so we can "discard" the whole row. 


<p align="center">
<img src="https://github.com/Sebi-Lopez/A-Star_Optimizations-Research/blob/master/docs/images/JPS/Prunning%20Examples/pruningHorizontalExample07.PNG?raw=true" width="200">
</p>

There's a trick. What happens when the nodes that I assumed that will be analyzed by other jumps, are blocked? Then takes place what it's called a Forced Neighbour. I have, then, to keep that in mind add myself as a JumpPoint in the open list. A Jump Point is a node that is interesting to look at, and it has this name because we can directly go to that node, ignoring all the others in the way, as they secured that there are no more interesting points to look at there. 

As i mentioned before, we only stop when we find an obstacle, but now that we know about Forced Neighbour, that's also a stop call. So, in conclusion: we keep jumping in that direction until we find with a Forced Neighbour (then I become a Jump Point and add myself to the open list) or when we find a wall. 


<p align="center">
<img src="https://github.com/Sebi-Lopez/A-Star_Optimizations-Research/blob/master/docs/images/JPS/Prunning%20Examples/pruningHorizontalExample06.PNG?raw=true" width="200">
</p>


Why is this an interesting point, you may ask. The reason is that this node can be a node that belongs to the optimal path, and we assured, that the "best" way to get to it, is going through me (adding myself in the open list to be analyzed). 

These conditions of encountering forced neighbours are applied in a very similar way to the vertical jumps, so I won't get deep in those. 

### Diagonal Jumps 

For the diagonal jumps we make similar assumptions as with the horizontal and vertical jumps to find Jump Points. Also, it has a peculiarity in its expansion, that I am going to explain a bit further. 

If I go, let's say in this direction, 
<p align="center">
<img src="https://github.com/Sebi-Lopez/A-Star_Optimizations-Research/blob/master/docs/images/JPS/Prunning%20Examples/pruningDiagonalExample01.PNG?raw=true" width="200">
</p>

I can assume that the nodes above and to the right of my parent are reached better through it rather than going through me. 

<p align="center">
<img src="https://github.com/Sebi-Lopez/A-Star_Optimizations-Research/blob/master/docs/images/JPS/Prunning%20Examples/pruningDiagonalExample02.PNG?raw=true" width="200">
</p>

These two nodes in my corners are, as well, reached more optimally through my parent. As going through me it would suppose a movement of 2√2 and going through my parent it would be only 2. 

<p align="center">
<img src="https://github.com/Sebi-Lopez/A-Star_Optimizations-Research/blob/master/docs/images/JPS/Prunning%20Examples/pruningDiagonalExample03.PNG?raw=true" width="200">
</p>

So now I am left with these three directions, that are all "ahead" of me. How do I prune all these directions? 

<p align="center">
<img src="https://github.com/Sebi-Lopez/A-Star_Optimizations-Research/blob/master/docs/images/JPS/Prunning%20Examples/pruningDiagonalExample04.PNG?raw=true" width="200">
</p>


Here comes the trick: for each node that we are jumping over diagonally, before we move on to the next diagonal node, I have to prune the vertical and horizontal directions.  So, when I am doing a diagonal jump, I first jump horizontally do the right (in this case). If this jump doesn't returns me a Jump Point, i can safely discard that row, and I proceed to do a vertical jump. Again if this jump doesn't tell me that there is a Jump Point in that row, I can discard it safely. That's almost the main part of these jumping methods, as it expands in all the possible directions doing this method, as shown in the image below. 

<p align="center">
<img src="https://github.com/Sebi-Lopez/A-Star_Optimizations-Research/blob/master/docs/images/JPS/Strategy.PNG?raw=true" width="200">
</p>

Now, let's not forget, that we are assuming, when doing all this, that the rows that we assumed that are free and that some other path will go through them, may have an obsticle in their way. In this case, as with the horizontal and vertical jumps, we have a Jump Point. So I have to add myself to the open list, to be analyzed later on. 

<p align="center">
<img src="https://github.com/Sebi-Lopez/A-Star_Optimizations-Research/blob/master/docs/images/JPS/Prunning%20Examples/pruningDiagonalExample05.PNG?raw=true" width="200">
</p>

Like in the horizontal and vertical jumps, the jump ends either when we find a Jump Point (either via vertical or horizontal jump or our forced neighbours) or when we find with an obstacle (meaning that we can not jump any further). 

We also have to consider, that when we get to the goal node, we also have to stop, as we ended our search. 

## Iterating
Once explained the way that pruning doing jumps works, let's see how we work with the actual Jump Points that these jumps give to the open list. 
The A* takes the lowest node in his open list and finds all his walkable adjacents. Jump Point Search, does practically the same, with the only difference that it prunes them before adding them to the neighbour list. This way, we can discard many nodes, that are not interesting as we checked with our prunning method. Furthermore, when a Jump Point is added to the open list, it's because it found either the goal (in which case our job is done), or because it found a Forced Neighbour. Expanding in all directions (in the directions of my walkable adjacents) we make sure that this Forced Neighbour is explored aswell. "

This is the only thing that changes the JPS in the A* method. Once we have done this, we are all set. 

## Implementation

### Disclaimer 
I missunderstood the iteration of the JPS initally, and when the pruning methods where applied to each node. That's why I decided to work in another I've recently discovered that many of the things I did were done faster and better with the JPS, as expected. 


## Exercises
### TODO 0: 
"Add 8 different starting nodes, with the 8 possible directions to the open list."
To start things off, we need to be able to expand in all the possible directions. 

- Solution

### TODO 1: 
"Fill the nieghbours list with the pruned neighbours. Keep in mind that we do the same like in A*, only that we prune before adding elements. It's a single line."

´´´cpp 

// Horizontal Cases
// East
open.pathNodeList.push_back(PathNode(0, origin.DistanceManhattan(goal), origin, nullptr, { 1,0 }));
// West
open.pathNodeList.push_back(PathNode(0, origin.DistanceManhattan(goal), origin, nullptr, { -1,0 }));

// VERTICAL CASES 
// North
open.pathNodeList.push_back(PathNode(0, origin.DistanceManhattan(goal), origin, nullptr, { 0, 1 }));
// South
open.pathNodeList.push_back(PathNode(0, origin.DistanceManhattan(goal), origin, nullptr, { 0, -1 }));

// DIAGONAL CASES 
// North - East
open.pathNodeList.push_back(PathNode(0, origin.DistanceManhattan(goal), origin, nullptr, { 1, 1 }));
// South - East
open.pathNodeList.push_back(PathNode(0, origin.DistanceManhattan(goal), origin, nullptr, { 1, -1 }));
// South - West
open.pathNodeList.push_back(PathNode(0, origin.DistanceManhattan(goal), origin, nullptr, { -1, -1 }));
// North - West
open.pathNodeList.push_back(PathNode(0, origin.DistanceManhattan(goal), origin, nullptr, { -1, 1 }));

´´´

###TODO 2: 

### TODO 3: 

### TODO 4: 

### TODO 5: 

### TODO 6: 


## Improvements: 
## Don't miss any nodes

## RSR
 RSR or Rectangular Symmetry Reduction is another pre-processing algorithm that avoids path symmetries by dividing the map grid into different rectangles. The idea is to dodge path symmetry by avoiding all the centre nodes in those rectangles, and only expanding nodes from the perimeters of each rectangle. It's created by Don Harabor as well, the creator of JPS. The combination of these two methods, as he shows in his paper, can speed up the search by. 

<p align="center">
<img src="https://github.com/Sebi-Lopez/A-Star_Optimizations-Research/blob/master/docs/images/RSR/rsr_decomposition.png?raw=true" width="340">
</p>

<p align="center">
<img src="https://github.com/Sebi-Lopez/A-Star_Optimizations-Research/blob/master/docs/images/RSR/rsr_insertion.png?raw=true" width="340">
</p>


