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

## Description
Its main purpose is to reduce the number of nodes in the open list of the A*. This optimizes the search speed for two reasons. 
1 - It reduces the number of operations needed to make a path.
2 - (Most Important) With fewer nodes in the open list every iteration to find the node with lower cost is cheaper. 

How does it do it? Well, the concept that you have to stick to your head is Path Symmetry.  

### Path Symmetry
His creator, Daniel Harabor, has made an incredible optimization of the A* by exploiting the path symmetry. The idea is that the A* algorithm looks through lots of similar paths that are symmetric, most of all in larger and open spaces. As you can see in the picture below, all those paths are equivalent when we talk about efficiency.  The only difference between one and another is which direction you take first. At the end of the day, you will do the same movements, in a different order.



When talking about symmetric paths, the A* algorithm is forced to explore every node adjacent to the optimal path. In the past picture, depending on how we handle the situation of having two nodes with the same score, A* might even explore the whole map before reaching the destination. 


## Jumping 
The principal idea is that there is no need to explore every possible path (since most of them are symmetric), so not every node is interesting to look at. The algorithm "jumps over" these non-interesting nodes, avoiding to analyze them explicitly (adding them to the open list). It does this following two jump rules, also called Pruning Rules. 
Another way to look at what it does is saying that each jump, tries to "prove" that exists another path to the goal that is equally optimal (symmetric) and doesn't pass through certain nodes. Is a bit twisted, but you'll see it clearer now. 

## Pruning Rules
There are two main rules for pruning. These two are separated only by the direction of the jump we are trying to make. We differentiate between straight jumps and diagonal jumps. 

Horizontal Jumps: 




## Iterating
## Exercises


## Improvements: 

## Don't miss any nodes

## RSR
 RSR or Rectangular Symmetry Reduction is another pre-processing algorithm that avoids path symmetries by dividing the map grid into different rectangles. The idea is to dodge path symmetry by avoiding all the centre nodes in those rectangles, and only expanding nodes from the perimeters of each rectangle. It's created by Don Harabor as well, the creator of JPS. The combination of these two methods, as he shows in his paper, can speed up the search by. 



