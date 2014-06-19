#include "ND_Interpolation_Functions.h"
#include <math.h>
#include <vector>
#include <stdlib.h>
#include "MDreader.h"
#include <iostream>


InverseDistanceWeighting::InverseDistanceWeighting(std::string filename, double p){
	_data_filename = filename;
	int dimensions;
	int numberOfPoints;
	std::vector<double> values;
	std::vector< std::vector<double> > pointCoordinates;

	readScatteredNDarray(_data_filename, dimensions, numberOfPoints, pointCoordinates, values);

	_dimensions = dimensions;
	_number_of_points = numberOfPoints;
	_values = values;
	_point_coordinates = pointCoordinates;
	_p = p;
	_completed_init = true;

	std::cerr << "_dimensions " << _dimensions << std::endl;
	std::cerr << "_number_of_points " << _number_of_points << std::endl;
}

InverseDistanceWeighting::InverseDistanceWeighting(double p){
    _p = p;
    _completed_init = false;
    _number_of_points = 0;
    _dimensions = 0;
}

double InverseDistanceWeighting::interpolateAt(std::vector<double> point){
	double value = 0;
	double weightsCumulativeSum = 0;
	std::vector<double> weights (_number_of_points);

	if (not _completed_init)
	{
	  throw ("Error in interpolateAt: the class has not been completely initialized... you can not interpolate!!!!");
	}
	for (int i=0; i<_number_of_points; i++){
		if (minkowskiDistance(point, _point_coordinates[i],_p) == 0){
			value = _values[i];
			weightsCumulativeSum = 1;
			break;
		} else {
			weights[i]= pow(1.0/minkowskiDistance(point, _point_coordinates[i],_p),_dimensions+1);
			weightsCumulativeSum += weights[i];
			value += weights[i] * _values[i];
		}
	}

	value = value/weightsCumulativeSum;

	return value;
}

double InverseDistanceWeighting::getGradientAt(std::vector<double> point){
	// TO BE COMPLETED
  if (not _completed_init)
  {
    throw ("Error in getGradientAt: the class has not been completely initialized... you can not interpolate!!!!");
  }
  double gradient= -1;
  return gradient;
}

void
InverseDistanceWeighting::fit(std::vector< std::vector<double> > coordinates, std::vector<double> values){
  _dimensions=coordinates[0].size();
  _number_of_points = coordinates.size();
  _point_coordinates = coordinates;
  _values = values;
  _completed_init = true;
}
