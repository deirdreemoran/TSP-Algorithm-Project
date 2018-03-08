#!/usr/local/bin/python

#*****************************************************************************************
# Program Filename: TSP.py
# Group 52: Deirdre Moran, Joel Tennyson
# Date: 1 December, 2017
# Description: This program reads city data from the input file specified on the command
# line, then determines a TSP/Hamiltonion tour through the cities so that each city is
# visited exactly once. The order of cities to visit and total distance cost of that tour
# is output to a file of the same name as the input file, with .tour appended.
# Usage: python TSP.py tsp_example_1.txt
#*****************************************************************************************
import sys
import math

# This class allows city objects to store their unique identifier, x coord, and y coord
class City(object):
    identifier = -1
    x = -1
    y = -1
    adjList = []
    eulList = []

    def __init__(self, identifier, x, y):
        self.identifier = int(identifier)
        self.x = int(x)
        self.y = int(y)
        self.adjList = []
        self.eulList = []

#Holds adjacent vectors as objects
class subCity(object):
    city1 = -1
    city2 = -1
    dist = -1
    visited = "no"

    def __init__(self, city1, city2, dist):
        self.city1 = int(city1)
        self.city2 = int(city2)
        self.dist = int(dist)

# This function reads city data from the specified input file and outputs a list of
# City objects with data members: city identifier, x coord, and y coord
def readCities():
    cities = []
    # open the desired file for reading
    inputFile = open(sys.argv[1], "r")
    # read each line in the file into a list, inputData
    inputData = inputFile.read().splitlines()
    # close the file
    inputFile.close()
    # for each city we have data about
    for city in inputData:
        cityParsed = city.split()
        # add an appropritate City object to the list of city objects
        cities.append(City(cityParsed[0], cityParsed[1], cityParsed[2]))
    return(cities)


# This function returns the distance between the two cities passed to it
def calcDistance(city1, city2):
    # Distance is calculated based on the cities' coordinates
    distance = math.sqrt(((city1.x - city2.x)**2)+((city1.y - city2.y)**2))
    # The distance is rounded to the nearest integer value before being returned
    return(int(round(distance)))


#A = adjacency matrix, u = vertex u, v = vertex v
def weight(A, u, v):
    return A[u][v]

#A = adjacency matrix, u = vertex u
def adjacent(A, u):
    L = []
    for x in range(len(A)):
        if A[u][x] > 0 and x <> u:
            L.insert(0,x)
    return L

#Q = min queue
def extractMin(Q):
    q = Q[0]
    Q.remove(Q[0])
    return q

#Q = min queue, V = vertex list
def decreaseKey(Q, K):
    for i in range(len(Q)):
        for j in range(len(Q)):
            if K[Q[i]] < K[Q[j]]:
                s = Q[i]
                Q[i] = Q[j]
                Q[j] = s


def prim(V, A, r):
    u = 0
    v = 0
    # initialize and set each value of the array P (pi) to none
    # pi holds the parent of u, so P(v)=u means u is the parent of v
    P=[None]*len(V)

    # initialize and set each value of the array K (key) to some large number (simulate infinity)
    K = [999999]*len(V)

    # initialize the min queue and fill it with all vertices in V
    Q=[0]*len(V)
    for u in range(len(Q)):
        Q[u] = V[u]

    # set the key of the root to 0
    K[r] = 0
    decreaseKey(Q, K)    # maintain the min queue

    # loop while the min queue is not empty
    while len(Q) > 0:
        u = extractMin(Q)    # pop the first vertex off the min queue

        # loop through the vertices adjacent to u
        Adj = adjacent(A, u)
        for v in Adj:
            w = weight(A, u, v)    # get the weight of the edge uv

            # proceed if v is in Q and the weight of uv is less than v's key
            if Q.count(v)>0 and w < K[v]:
                # set v's parent to u
                P[v] = u
                # v's key to the weight of uv
                K[v] = w
                decreaseKey(Q, K)    # maintain the min queue
    return P


if __name__ == '__main__':

    cities = readCities()
    mylist = []
    for city1 in cities:
        for city2 in cities:
            distance = math.sqrt(((city1.x - city2.x)**2)+((city1.y - city2.y)**2))
            # The distance is rounded to the nearest integer value before being returned
            p = int(round(distance))
            city1.adjList.append(subCity(city1.identifier, city2.identifier, p))
           # city.adjList.dist.append(calcDistance(city1, city2))

    A = [[0] * len(cities) for i in range(len(cities))]
    i = 0
    j = 0
    for city in cities:
        for i in range(0, len(cities)):
            A[i][j] = city.adjList[i].dist
        j = j + 1

    #SO NOW ALL CITIES HAVE ADJACENCY LISTS THAT ARE SORTED BY DISTANCE
    V = []
    for city in cities:
        V.append(city.identifier)

    #PRIM'S MINIMUM SPANNING TREE
    P = prim(V, A, 0)
    print P
    print P[0]
    print(len(P))
    #Duplicate all edges, even degree
    idx = 0
    for i in range(0, len(cities)): #cit1, cit2, dist,
        if(i == 0):
            distance = 0
            cities[i].eulList.append(subCity(i, -1, p))
            cities[i].eulList.append(subCity(i, -1, p))
        else:
            distance = math.sqrt(((cities[i].x - cities[P[i]].x)**2)+((cities[i].y - cities[P[i]].y)**2))
            p = int(round(distance))
            cities[i].eulList.append(subCity(i, P[i], p))
            cities[P[i]].eulList.append(subCity(P[i], i, p))
            cities[i].eulList.append(subCity(i, P[i], p))
            cities[P[i]].eulList.append(subCity(P[i], i, p))
    for city in cities:
        print('---')
        for var in city.eulList:
            print var.city2



    #get a Euler cycle from the graph, which is





