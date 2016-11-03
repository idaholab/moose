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
  InterpolationFunctions(std::vector<double> x_coordinates, std::vector<double> y_coordinates, custom_dist_fit_type type);

  virtual ~InterpolationFunctions();

  double interpolation (double x_point);
  double cumulativeInterpolation (double x_point);

protected:
  LinearInterpolation  _linear;
  SplineInterpolation  _spline;
  std::vector<double>  _x_coordinates;
  std::vector<double>  _y_coordinates;
  custom_dist_fit_type _type;

  double interpolationStepLeft (double x_point);
  double interpolationStepRight (double x_point);

  double cumulativeInterpolationStepLeft (double x_point);
  double cumulativeInterpolationStepRight (double x_point);
  double cumulativeInterpolationLinear (double x_point);
};



#endif /* INTERPOLATION_FUNCTIONS_H_ */
