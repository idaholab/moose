/*
 * InterpolationFunctions.h
 *
 *  Created on: Jul 10, 2012
 *      Author: alfoa
 */

#ifndef INTERPOLATION_FUNCTIONS_H_
#define INTERPOLATION_FUNCTIONS_H_

#include <vector>
#include "LinearInterpolation.h"
#include "SplineInterpolation.h"

enum custom_dist_fit_type {STEP_LEFT, STEP_RIGHT, LINEAR, CUBIC_SPLINE};

class InterpolationFunctions{
public:
  InterpolationFunctions();
  InterpolationFunctions(std::vector<double> x_coordinates, std::vector<double> y_cordinates, custom_dist_fit_type type);

  virtual ~InterpolationFunctions();

  double interpolation (double x_point);
  double cumulativeInterpolation (double x_point);

protected:
  LinearInterpolation  _linear;
  SplineInterpolation  _spline;
  std::vector<double>  _x_coordinates;
  std::vector<double>  _y_coordinates;
  custom_dist_fit_type _type;

  double interpolation_Step_Left (double x_point);
  double interpolation_Step_Right (double x_point);

  double cumulativeInterpolation_Step_Left (double x_point);
  double cumulativeInterpolation_Step_Right (double x_point);
  double cumulativeInterpolation_Linear (double x_point);
};



#endif /* INTERPOLATION_FUNCTIONS_H_ */
