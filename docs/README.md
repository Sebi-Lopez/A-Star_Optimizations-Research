# Introduction
I am [Sebastià López Tenorio](https://github.com/Sebi-Lopez), student of the [Bachelor’s Degree in Video Games by UPC at CITM](https://www.citm.upc.edu/ing/estudis/graus-videojocs/). This content is generated for the second year’s subject Project 2, under the supervision of lecturer [Ricard Pillosu](https://es.linkedin.com/in/ricardpillosu)

# Current Position

In videogames, the use of a pathfinding algorithm is crucial when dealing with all sorts of movements. 
The algorithm that we are using now, the A*, as many of you know, can be really slow. Most of all when we have to deal with a larger map. At some point, the number of nodes that this algorithm has to explore and look through make it unusable. Either it takes too long to give a path (making unities look unresponsive) or the FPS of the game drop (disaster). This problem grows as well with the number of units demanding path. 

It's for this reason we are going to find a way we can improve this algorithm. 

# Possible Optimizations
There are lots of possibilities to optimize the A* each one with its pros and cons. 

## Parallel Search


## Potential Fields


## Lifelong Planning A*


## Theta*


## ARA* (Anytime Repairing A*)


## HPA* 
Hierarchical Path-Finding A* is a pathfinding method that abstracts any grid-map to different sets of "clusters" or "blocks" with different levels of abstraction. In their [paper](https://webdocs.cs.ualberta.ca/~mmueller/ps/hpastar.pdf) the creators use a metaphor with a car trip starting in one city and ending in a city in another country (both points with its respective addresses). Us, humans, can abstract the path we are going to take really well. We first look to get into a highway, then move from one state to another, and when we get to the destination city or state, we search at the "city level", within its streets and roundabouts. This method follows this abstraction principle. 
As mentioned, each cluster has information about its entries, its distances and costs. So travelling at "city level" can get very efficient. 

## IDA* 

## Dead End Detection
## Dead End Heuristics
## Swamps
El uso de pantanos es otra posible optimizacion que intenta evitar areas que son navegadas i extendidas innecesariamente. De esta forma, evitan la propagacion por zonas (que pueden llegar a ser muy grandes) en las cuales sabemos que el path optimo no cruzarà. 
The usage of Swamps is another method that tries to avoid areas that are navigated unnecessary by heuristic methods like A*. It calculates zones this undesirable zones in pre-runtime. This way, avoid the expansion of the nodes in zones (that can get to be really big) in which we know the path won't pass. In fact, the path will only pass those zones if the end of the beginning of it is located in these zones. 

## Portal Heuristic


## RSR
 RSR or Rectangular Symmetry Reduction is another pre-processing algorithm that avoids path symmetries by dividing the map grid into different rectangles. The idea is to dodge path symmetry by avoiding all the centre nodes in those rectangles, and only expanding nodes from the perimeters of each rectangle. 


# Selected Approach: JPS
Jump Point S
## Description
## Pruning Rules
## Iterating
## Exercices




