/*
 * InterpolationFunctions.C
 *
 *  Created on: Jul 10, 2012
 *      Author: alfoa
 */

#include "InterpolationFunctions.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>

InterpolationFunctions::InterpolationFunctions()
{
}

InterpolationFunctions::InterpolationFunctions(std::vector<double> x_cordinates, std::vector<double> y_cordinates, custom_dist_fit_type type):
  _x_coordinates(x_cordinates),
  _y_coordinates(y_cordinates),
  _type(type)
{
  if(_type == CUBIC_SPLINE){
    _spline=SplineInterpolation(_x_coordinates,_y_coordinates,1e30,1e30);
  }
  else if(_type == LINEAR){
    _linear=LinearInterpolation(_x_coordinates,_y_coordinates);
  }
}


InterpolationFunctions::~InterpolationFunctions(){

}

double
InterpolationFunctions::interpolation(double x_point){
    double interp_value;

    switch (_type) {
      case STEP_LEFT:
        interp_value = interpolation_Step_Left(x_point);
        break;
      case STEP_RIGHT:
        interp_value = interpolation_Step_Right(x_point);
        break;
      case LINEAR:
          interp_value = _linear.sample(x_point);
        break;
      case CUBIC_SPLINE:
          interp_value = _spline.sample(x_point);
          break;
      default:
          interp_value = -10000000000;
          break;
    }
    return interp_value;
}

double
InterpolationFunctions::interpolation_Step_Left (double x_point){

  int pivotPoint=0;

  for (unsigned int i=0; i<_x_coordinates.size(); i++){
    if (x_point>=_x_coordinates[i]) {
        pivotPoint=i;
    }
  }

  return _y_coordinates.at(pivotPoint);
}

double
InterpolationFunctions::interpolation_Step_Right (double x_point){
  int pivotPoint=0;

  for (unsigned int i=0; i<_x_coordinates.size()-1; i++){
    if (x_point>_x_coordinates[i]) {
      pivotPoint=i;
    }
  }

  return _y_coordinates.at(pivotPoint+1);
}

double
InterpolationFunctions::cumulativeInterpolation(double x_point){
    double interp_value;

    switch (_type) {
      case STEP_LEFT:
        interp_value = cumulativeInterpolation_Step_Left(x_point);
        break;
      case STEP_RIGHT:
        interp_value = cumulativeInterpolation_Step_Right(x_point);
        break;
      case LINEAR:
          interp_value = cumulativeInterpolation_Linear(x_point);
        break;
      default:
          interp_value = -10000000000;
          break;
    }
    return interp_value;
}

double
InterpolationFunctions::cumulativeInterpolation_Step_Left(double x_point){
  double value;

  std::vector<double> CDF (_x_coordinates.size());
  CDF[0]=0;

  for (unsigned int i=1; i<_x_coordinates.size(); i++)
    CDF[i]=CDF[i-1]+_x_coordinates[i];

  int lower; //upper
  std::vector<double>::iterator it;

  it=std::lower_bound(_x_coordinates.begin() , _x_coordinates.end() , x_point);

  lower=(it-1) - _x_coordinates.begin();
  //upper=*it;

  value = CDF[lower];

  return value;
}

double
InterpolationFunctions::cumulativeInterpolation_Step_Right(double x_point){
  double value;

  std::vector<double> CDF (_x_coordinates.size());
  CDF[0]=0;

  for (unsigned int i=1; i<_x_coordinates.size(); i++)
    CDF[i]=CDF[i-1]+_x_coordinates[i];

  int upper; //,lower;
  std::vector<double>::iterator it;

  it=std::lower_bound(_x_coordinates.begin() , _x_coordinates.end() , x_point);

  //lower=*(it-1);
  upper=it - _x_coordinates.begin();

  value = CDF[upper];

  return value;
}

double
InterpolationFunctions::cumulativeInterpolation_Linear(double x_point){
  double value;

  std::vector<double> CDF (_x_coordinates.size());
  CDF[0]=0;

  for (unsigned int i=1; i<_x_coordinates.size(); i++)
     CDF[i] = CDF[i-1] + 1/2 * (_y_coordinates[i]-_y_coordinates[i-1]) * (_x_coordinates[i]-_x_coordinates[i-1]);

  int upper,lower;
  std::vector<double>::iterator it;
  it=std::lower_bound(_x_coordinates.begin() , _x_coordinates.end() , x_point);

  lower=(it-1) - _x_coordinates.begin();
  upper=it - _x_coordinates.begin();

  value = CDF[lower] + (_y_coordinates[upper]-_y_coordinates[lower]) / (_x_coordinates[upper]-_x_coordinates[lower]) * (x_point-_x_coordinates[lower]);

  return value;
}
