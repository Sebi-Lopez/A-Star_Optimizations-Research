# Introduction
I am [Sebastià López Tenorio](https://github.com/Sebi-Lopez), student of the [Bachelor’s Degree in Video Games by UPC at CITM](https://www.citm.upc.edu/ing/estudis/graus-videojocs/). This content is generated for the second year’s subject Project 2, under the supervision of lecturer [Ricard Pillosu](https://es.linkedin.com/in/ricardpillosu)

# Current Position

In videogames, the use of a pathfinding algorithm is crucial when dealing with all sorts of movements. 
The algorithm that we are using now, the A*, as many of you know, can be really slow. Most of all when we have to deal with a larger map. At some point, the number of nodes that this algorithm has to explore and look through make it unusable. Either it takes too long to give a path (making unities look unresponsive) or the FPS of the game drop (disaster). This problem grows as well with the number of units demanding path at the same time. 

It's for this reason we are going to find a way we can improve this algorithm. 

# Possible Optimizations
There are lots of possibilities to optimize the A* each one with its pros and cons. I'm not going to go through all of them, but I'm going to try to briefly explain the main mechanics of some that I found interesting to take a look at. 

## Parallel Search


## Potential Fields


## Lifelong Planning A*


## Theta*


## ARA* (Anytime Repairing A*)


## HPA* 
This algorithm has taken over the game industry. Is one of the most used in the present. 
Hierarchical Path-Finding A* is a pathfinding method that abstracts any grid-map to different sets of "clusters" or "blocks" with different levels of abstraction. In their [paper](https://webdocs.cs.ualberta.ca/~mmueller/ps/hpastar.pdf) the creators use a metaphor with a car trip starting in one city and ending in a city in another country (both points with its respective addresses). Us, humans, can abstract the path we are going to take really well. We first look to get into a highway, then move from one state to another, and when we get to the destination city or state, we search at the "city level", within its streets and roundabouts. This method follows this abstraction principle. 

As mentioned, each cluster has information about its entries, its distances and costs. So travelling at "city level" can get very efficient. These clusters are made a clustering algorithm that groups neighbours together when appropriate. Therefore, there's no need for, let's say, the designer of the map, to add extra data when creating the map, as this algorithm makes the abstraction zones by itself. Consequently, this method doesn't have any problem to be added to a procedurally generated map.

## IDA* 

## Dead End Detection
## Dead End Heuristics
## Swamps
El uso de pantanos es otra posible optimizacion que intenta evitar areas que son navegadas i extendidas innecesariamente. De esta forma, evitan la propagacion por zonas (que pueden llegar a ser muy grandes) en las cuales sabemos que el path optimo no cruzarà. 
The usage of Swamps is another method that tries to avoid areas that are navigated unnecessary by heuristic methods like A*. It calculates zones this undesirable zones in pre-runtime. This way, avoid the expansion of the nodes in zones (that can get to be really big) in which we know the path won't pass. In fact, the path will only pass those zones if the end of the beginning of it is located in these zones. 

## Portal Heuristic





# Selected Approach: JPS
The  [Jump Point Search](http://users.cecs.anu.edu.au/~dharabor/data/papers/harabor-grastien-aaai11.pdf) is an algorithm build upon A* pathfinding algorithm and works in uniform-cost grid maps. It requires no preprocessing nor occupies memory (unlike most of the other optimizations) and it's compatible with other improving technics like abstraction.

I'm going to explain its functionality, and afterwards, I'm going to explain the way I implemented it, guided by this [website](https://www.gamedev.net/articles/programming/artificial-intelligence/jump-point-search-fast-a-pathfinding-for-uniform-cost-grids-r4220/) that approaches the method in a different way. I made comparisons with both approaches and concluded that this one has better results. 

## Description
Its main purpose is to reduce the number of nodes in the open list of the A*. This optimizes the search speed for two reasons. 
1 - It reduces the number of operations needed to make a path.
2 - (Most Important) With fewer nodes in the open list every iteration to find the node with lower cost is cheaper. 

How does it do it? Well, the concept that you have to stick to your head is Path Symmetry.  

### Path Symmetry
His creator, Daniel Harabor, has made an incredible optimization of the A* by exploiting the path symmetry. The idea is that the A* algorithm looks through lots of similar paths that are symmetric, most of all in larger and open spaces. As you can see in the picture below, all those paths are equivalent when we talk about efficiency.  The only difference between one and another is which direction you take first. At the end of the day, you will do the same movements, in a different order.

<p align="center">
<img src="https://github.com/Sebi-Lopez/A-Star_Optimizations-Research/blob/master/docs/images/JPS/FPSB_symmetry.png" width="640">
</p>

<img src="https://github.com/Sebi-Lopez/A-Star_Optimizations-Research/blob/master/docs/images/JPS/FPSB_symmetry.png?raw=true" width="200"/>

<img src="https://github.com/Sebi-Lopez/A-Star_Optimizations-Research/blob/master/docs/images/JPS/Symmetrt.PNG?raw=true"/>

<img src="https://github.com/Sebi-Lopez/A-Star_Optimizations-Research/blob/master/docs/images/JPS/symmetry3.PNG?raw=true"/>

When talking about symmetric paths, the A* algorithm is forced to explore every node adjacent to the optimal path. In the past picture, depending on how we handle the situation of having two nodes with the same score, A* might even explore the whole map before reaching the destination. 


## Jumping 
The principal idea is that there is no need to explore every possible path (since most of them are symmetric), so not every node is interesting to look at. The algorithm "jumps over" these non-interesting nodes, avoiding to analyze them explicitly (adding them to the open list). It does this following two jump rules, also called Pruning Rules. 
Another way to look at what it does is saying that each jump, tries to "prove" that exists another path to the goal that is equally optimal (symmetric) and doesn't pass through certain nodes. Is a bit twisted, but you'll see it clearer now. 

## Pruning Rules
There are two main rules for pruning. These two are separated only by the direction of the jump we are trying to make. We differentiate between straight jumps (horizontal and vertical) and diagonal jumps. 

To discard the major number of nodes that we are not interested, we are going to look at it through the perspective, am I (as the node) really needed to be on the final path. To prove so, we are going to make sure, that there is no other "interesting" point that needs to be analyzed that forces the path to go through me (because I am part of the optimal path to get to that node). We will talk about this interesting nodes after explaining the jumps. 

### Horizontal Jumps: 
So I (again as a node) am trying to jump over the right direction (as my parent is on my left). 

I can assume these two nodes, above and below of my parent, don't need to go through me to get there (they can easily go through my parent). 

As well, I can assume that the nodes avobe and below me, are optimally obtained going through my parent diagonal (as its moving distance  is √2 and going through me the distance would be 2). 

To get to the nodes diagonally more to the right of me, the optimal path can go through me, but as well, it can go through the two nodes that we just discarded, as the path is symmetric. We are going to assume, that that is the correct path to go (you will see why later).

So now, we only have one direction to go, as all the others will be explored and analyzed by other jumps. So I will keep jumping nodes horizontally to the right until I encounter with a wall. Then my jump will be over and I can guarantee that there are no interesting points that need to pass through that row of nodes, so we can "discard" the whole row. 

There's a trick. What happens when the nodes that I assumed that will be analyzed by other jumps, are blocked? Then takes place what it's called a Forced Neighbour. I have, then, to keep that in mind add myself as a JumpPoint in the open list. A Jump Point is a node that is interesting to look at, and it has this name because we can directly go to that node, ignoring all the others in the way, as they secured that there are no more interesting points to look at there. 

Why is this an interesting point, you may ask. The reason is that this node can be a node that belongs to the optimal path, and we assured, that the "best" way to get to it, is going through me (adding myself in the open list to be analyzed). 

These conditions of encountering forced neighbours are applied in a very similar way to the vertical jumps, so I won't get deep in those. 

### Diagonal Jumps 



## Iterating
Once explained the way we can safely discard a large number of nodes (bigger the more big and free the map is) let's see how we apply this technic: 

For every Jump Point 



## My take on it


## Exercises
### TODO 1: 

###TODO 2: 

### TODO 3: 

### TODO 4: 

### TODO 5: 

### TODO 6: 


## Improvements: 
## Don't miss any nodes

## RSR
 RSR or Rectangular Symmetry Reduction is another pre-processing algorithm that avoids path symmetries by dividing the map grid into different rectangles. The idea is to dodge path symmetry by avoiding all the centre nodes in those rectangles, and only expanding nodes from the perimeters of each rectangle. It's created by Don Harabor as well, the creator of JPS. The combination of these two methods, as he shows in his paper, can speed up the search by. 




