#include "ND_Interpolation_Functions.h"
#include <math.h>
#include <vector>
#include <stdlib.h>
#include "MDreader.h"
using namespace std;
#include <iostream>


inverseDistanceWeigthing::inverseDistanceWeigthing(std::string filename, double p){
	_dataFileName = filename;
	int dimensions;
	int numberOfPoints;
	std::vector<double> values;
	std::vector< std::vector<double> > pointCoordinates;

	readScatteredNDarray(_dataFileName, dimensions, numberOfPoints, pointCoordinates, values);

	_dimensions = dimensions;
	_numberOfPoints = numberOfPoints;
	_values = values;
	_pointCoordinates = pointCoordinates;
	_p = p;
	_completedInit = true;

	std::cerr << "_dimensions " << _dimensions << std::endl;
	std::cerr << "_numberOfPoints " << _numberOfPoints << std::endl;
}

inverseDistanceWeigthing::inverseDistanceWeigthing(double p){
    _p = p;
    _completedInit = false;
    _numberOfPoints = 0;
    _dimensions = 0;
}

double inverseDistanceWeigthing::interpolateAt(std::vector<double> point){
	double value = 0;
	double weightsCumulativeSum = 0;
	std::vector<double> weights (_numberOfPoints);

	if (not _completedInit)
	{
	  throw ("Error in interpolateAt: the class has not been completely initialized... you can not interpolate!!!!");
	}
	for (int i=0; i<_numberOfPoints; i++){
		if (minkowskiDistance(point, _pointCoordinates[i],_p) == 0){
			value = _values[i];
			weightsCumulativeSum = 1;
			break;
		} else {
			weights[i]= pow(1.0/minkowskiDistance(point, _pointCoordinates[i],_p),_dimensions+1);
			weightsCumulativeSum += weights[i];
			value += weights[i] * _values[i];
		}
	}

	value = value/weightsCumulativeSum;

	return value;
}

double inverseDistanceWeigthing::getGradientAt(std::vector<double> point){
	// TO BE COMPLETED
  if (not _completedInit)
  {
    throw ("Error in getGradientAt: the class has not been completely initialized... you can not interpolate!!!!");
  }
  double gradient= -1;
  return gradient;
}

void
inverseDistanceWeigthing::fit(std::vector< std::vector<double> > coordinates, std::vector<double> values){
  _dimensions=coordinates[0].size();
  _numberOfPoints = coordinates.size();
  _pointCoordinates = coordinates;
  _values = values;
  _completedInit = true;
}
