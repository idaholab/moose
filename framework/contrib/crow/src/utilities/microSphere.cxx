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
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <iso646.h>
#include "MDreader.h"

MicroSphere::MicroSphere(std::string filename, double p, int precision){
 _data_filename = filename;
 _p = p;
 _precision = precision;

 readScatteredNDArray(_data_filename, _dimensions, _number_of_points, _point_coordinates, _values);

 msInitialization();
 _completed_init = true;
}

MicroSphere::MicroSphere(double p, int precision)
{
    _p = p;
    _precision = precision;
    _completed_init = false;
    _number_of_points = 0;
    _dimensions=0;
}

void MicroSphere::msInitialization(){
  std::srand (time(NULL));

 for (int j=0; j<_precision; j++){
  double sum = 0;
  do{
   for (int i=0; i<_dimensions; i++){
    // x,y,z are uniformly-distributed random numbers in the range (-1,1)
                          _unit_vector[j][i] = -1+2*std::rand();
    sum += _unit_vector[j][i];
   }
   sum = std::sqrt(sum);
  }while (sum > 1);

  for (int i=0; i<_dimensions; i++)
   _unit_vector[j][i] /= sum;
 }
}


void
MicroSphere::fit(std::vector< std::vector<double> > coordinates, std::vector<double> values){
  _dimensions=coordinates[0].size();
  _number_of_points = coordinates.size();
  _point_coordinates = coordinates;
  _values = values;
  msInitialization();
  _completed_init = true;
}


double MicroSphere::interpolateAt(std::vector<double> point_coordinate){
 double value = 0;
 double weight = 0;
 double cosValue = 0;
 if (not _completed_init)
 {
   throw ("Error in interpolateAt: the class has not been completely initialized... you can not interpolate!!!!");
 }
 std::vector<double> weights (_values.size());

 for (unsigned int n=0; n<_values.size(); n++){
  weight = minkowskiDistance(point_coordinate,_point_coordinates[n],_p);

  for (int j=0; j<_precision; j++){
   cosValue = cosValueBetweenVectors(_unit_vector[j],point_coordinate);

   if (cosValue*weight>0){
    // Do something
   }
  }
 }

 return value;
}

double MicroSphere::cosValueBetweenVectors(std::vector<double> point1, std::vector<double> point2){
 double cosAngle = 0;
 double point1point2 = 0;

 if (point1.size() == point2.size()){
  for (unsigned int nDim=0; nDim<point1.size(); nDim++){
   point1point2 += point1[nDim] * point2[nDim];
  }

  cosAngle = point1point2/(vectorNorm(point1, _p)*vectorNorm(point2, _p));
 }else
  throw ("Error in cosValueBetweenVectors: points having different dimensions");

 return cosAngle;
}

double MicroSphere::getGradientAt(std::vector<double> /* point_coordinate */){
  if (not _completed_init)
  {
    throw ("Error in getGradientAt: the class has not been completely initialized... you can not interpolate!!!!");
  }
  return -1.0;
}



