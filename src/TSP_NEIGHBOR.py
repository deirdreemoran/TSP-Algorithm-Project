#!/usr/local/bin/python

#*****************************************************************************************
# Program Filename: TSP_NEIGHBOR.py
# Group 52: Deirdre Moran, Joel Tennyson
# Date: 1 December, 2017
# Description: This program reads city data from the input file specified on the command
# line, then determines a TSP/Hamiltonion tour through the cities so that each city is
# visited exactly once. The order of cities to visit and total distance cost of that tour
# is output to a file of the same name as the input file, with .tour appended.
# Usage: python TSP_NEIGHBOR.py tsp_example_1.txt
#*****************************************************************************************

import sys
import math
import time

# This class allows city objects to store their unique identifier, x coord, and y coord
class City(object):

    def __init__(self, identifier, x, y):
        self.identifier = int(identifier)
        self.x = int(x)
        self.y = int(y)

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

# This function takes in a list of ordered cities for a tour and the total distance of that tour;
# it writes the data to the proper output file
def outputTour(tourPath, tourDistance):
    outputName = sys.argv[1] + '.tour'
    # Create (or open for appending) the output file
    outputFile = open(outputName, "a")
    # Write the total distance of the TSP tour
    outputFile.write(str(tourDistance))
    # Write the city identifiers in order of visitation
    for city in tourPath:
        outputFile.write('\n' + str(city.identifier))
    outputFile.write('\n')
    return()

# This function returns the distance between the two cities passed to it
def calcDistance(city1, city2):
    # Distance is calculated based on the cities' coordinates
    distance = math.sqrt(((city1.x - city2.x)**2)+((city1.y - city2.y)**2))
    # The distance is rounded to the nearest integer value before being returned
    return(int(round(distance)))

# This function uses the greedy nearest neighbor algorithm to find a path that satisfies the
# TSP problem requirements. At the start, all cities are unvisited.
def nearestNeighbor(cities):
    tourDistance = 0
    visited = []
    # We can start at any city; we'll choose the first in the list
    currentCity = cities[0]
    cities.remove(currentCity)
    visited.append(currentCity)
    bestEdge = float('inf')
    bestEdgeCity = 0
    # While there is at least one unvisited city or vertex
    while len(cities) > 0:
        # For each unvisited city
        for city in cities:
            # If the edge to the city is shorter than those previously assessed
            distance = calcDistance(currentCity, city)
            if 0 < distance < bestEdge:
                # Then that is the shortest edge assessed so far
                bestEdge = distance
                bestEdgeCity = city
        # At this point, bestEdge and bestEdgeCity refer to the city with the shortest distance from
        # the currently visited city. We will visit that city and add the distance to our total
        # tour distance.
        currentCity = bestEdgeCity
        cities.remove(bestEdgeCity)
        visited.append(bestEdgeCity)
        tourDistance = tourDistance + bestEdge
        bestEdge = float('inf')
        bestEdgeCity = 0
    # At this point, all cities have been visited once. The 'visited' list contains the city objects in the
    # order that they were visited, and 'tourDistance' holds the total cost of that TSP tour
    # Before returning, one more edge distance must be added to tourDistance to turn our TSP path into a TSP
    # tour: the cost of travelling from the final city visited back to the very first city visited
    tourDistance = tourDistance + calcDistance(visited[0], visited[-1])
    return(visited, tourDistance)


# Execution begins here
# Store the starting time
start_time = time.time()
# Get a list of city objects (with coords)
cities = readCities()
# Run the nearest neighbor greedy approximation algorithm to get a viable TSP tour
tourPath, tourDistance = nearestNeighbor(cities)
# Output the TSP solution data to the appropriate output file
outputTour(tourPath, tourDistance)
# Display the time taken for start to finish
taken_time = time.time() - start_time
print('TIME FROM START TO FINISH: %s seconds.' % (taken_time))