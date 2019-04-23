//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PiecewiseBase.h"

class Piecewise;

template <>
InputParameters validParams<Piecewise>();

/**
 * Function which provides a piecewise approximation to a provided
 * (x,y) point data set generated from file, (x,y) pairs, or separate
 * (x,y) vectors.  Derived classes which control the order
 * (constant, linear) of the approximation should be used directly.
 */
class Piecewise : public PiecewiseBase
{
public:
  Piecewise(const InputParameters & parameters);

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
};

