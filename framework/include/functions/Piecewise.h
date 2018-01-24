//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PIECEWISE_H
#define PIECEWISE_H

#include "Function.h"
#include "LinearInterpolation.h"

class Piecewise;

template <>
InputParameters validParams<Piecewise>();

/**
 * Function which provides a piecewise approximation to a provided
 * (x,y) point data set.  Derived classes which control the order
 * (constant, linear) of the approximation should be used directly.
 */
class Piecewise : public Function
{
public:
  Piecewise(const InputParameters & parameters);

  virtual Real functionSize();
  virtual Real domain(int i);
  virtual Real range(int i);

  /**
   * Provides a means for explicitly setting the x and y data.
   */
  void setData(const std::vector<Real> & x, const std::vector<Real> & y);

protected:
  /**
   * Reads data from supplied CSV file.
   */
  std::pair<std::vector<Real>, std::vector<Real>> buildFromFile();

  /**
   * Builds data from 'x' and 'y' parameters.
   */
  std::pair<std::vector<Real>, std::vector<Real>> buildFromXandY();

  /**
   * Builds data from 'xy_data' parameter.
   */
  std::pair<std::vector<Real>, std::vector<Real>> buildFromXY();

  const Real _scale_factor;
  std::unique_ptr<LinearInterpolation> _linear_interp;
  int _axis;
  bool _has_axis;
};

#endif
