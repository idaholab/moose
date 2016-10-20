#include "ND_Interpolation_Functions.h"
#include <vector>
#include <cmath>
#include "MDreader.h"
#include <iostream>


InverseDistanceWeighting::InverseDistanceWeighting(std::string filename, double p){
 _data_filename = filename;
 int dimensions;
 unsigned int numberOfPoints;
 std::vector<double> values;
 std::vector< std::vector<double> > pointCoordinates;

 readScatteredNDarray(_data_filename, dimensions, numberOfPoints, pointCoordinates, values);
 _dimensions = dimensions;
 _number_of_points = numberOfPoints;
 _values = values;
 _point_coordinates = pointCoordinates;
 _p = p;
 _completed_init = true;
 _cellPoint0.resize(dimensions);
 _cellDxs.resize(dimensions);

 // Functions do determine:
    //   * std::vector<double> _cellPoint0;
    //   * std::vector<double> _cellDxs;

 std::vector<double> cellPointInf;
 cellPointInf.resize(dimensions);

 for (int d=0; d<_dimensions; d++){
  _cellPoint0[d]  = _point_coordinates[0][d];
  cellPointInf[d] = _point_coordinates[0][d];
 }

 for (int n=1; n<_number_of_points; n++)
  for (int d=0; d<_dimensions; d++){
   if (_point_coordinates[n][d] < _cellPoint0[d])
    _cellPoint0[d] = _point_coordinates[n][d];
   if (_point_coordinates[n][d] > cellPointInf[d])
    cellPointInf[d] = _point_coordinates[n][d];
  }

 for (int d=0; d<_dimensions; d++)
  _cellDxs[d] = cellPointInf[d]-_cellPoint0[d];

 for (int i=0; i<_dimensions; i++){
     _lowerBound.push_back(_cellPoint0.at(i));
     _upperBound.push_back(_cellPoint0.at(i) + _cellDxs.at(i));
 }

 //std::cerr << "_dimensions " << _dimensions << std::endl;
 //std::cerr << "_number_of_points " << _number_of_points << std::endl;
 //std::cerr << "_p " << _p << std::endl;
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
  if (minkowskiDistance(point, _point_coordinates.at(i),_p) == 0.0){
   value = _values.at(i);
   weightsCumulativeSum = 1.0;
   break;
  } else {
   weights.at(i)= std::pow(1.0/minkowskiDistance(point, _point_coordinates.at(i),_p),_dimensions+1);
   weightsCumulativeSum += weights.at(i);
   value += weights.at(i) * _values.at(i);
  }
 }

 value = value/weightsCumulativeSum;

 return value;
}

double InverseDistanceWeighting::getGradientAt(std::vector<double> /* point */){
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
