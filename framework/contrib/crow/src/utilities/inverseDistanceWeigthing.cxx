/* Copyright 2017 Battelle Energy Alliance, LLC

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
#include "ND_Interpolation_Functions.h"
#include <vector>
#include <cmath>
#include "MDreader.h"
#include <iostream>
#include <iso646.h>


InverseDistanceWeighting::InverseDistanceWeighting(std::string filename, double p){
 _data_filename = filename;
 int dimensions;
 unsigned int number_of_points;
 std::vector<double> values;
 std::vector< std::vector<double> > point_coordinates;

 readScatteredNDArray(_data_filename, dimensions, number_of_points, point_coordinates, values);
 _dimensions = dimensions;
 _number_of_points = number_of_points;
 _values = values;
 _point_coordinates = point_coordinates;
 _p = p;
 _completed_init = true;
 _cell_point_0.resize(dimensions);
 _cell_dxs.resize(dimensions);

 // Functions do determine:
    //   * std::vector<double> _cell_point_0;
    //   * std::vector<double> _cell_dxs;

 std::vector<double> cellPointInf;
 cellPointInf.resize(dimensions);

 for (int d=0; d<_dimensions; d++){
  _cell_point_0[d]  = _point_coordinates[0][d];
  cellPointInf[d] = _point_coordinates[0][d];
 }

 for (int n=1; n<_number_of_points; n++)
  for (int d=0; d<_dimensions; d++){
   if (_point_coordinates[n][d] < _cell_point_0[d])
    _cell_point_0[d] = _point_coordinates[n][d];
   if (_point_coordinates[n][d] > cellPointInf[d])
    cellPointInf[d] = _point_coordinates[n][d];
  }

 for (int d=0; d<_dimensions; d++)
  _cell_dxs[d] = cellPointInf[d]-_cell_point_0[d];

 for (int i=0; i<_dimensions; i++){
     _lower_bound.push_back(_cell_point_0.at(i));
     _upper_bound.push_back(_cell_point_0.at(i) + _cell_dxs.at(i));
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
