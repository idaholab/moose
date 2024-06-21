//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "THMControl.h"

/**
 * Computes a moving average value of the input control with a user-specified
 * number of points to average. The output control value is named " name:value ",
 * where " name " is the name of the control object.
 **/

class SmootherControl : public THMControl
{
public:
  SmootherControl(const InputParameters & parameters);

  virtual void execute();
  static InputParameters validParams();

protected:
  /// Input data
  const Real & _input;
  /// The number of points to use in the moving average
  const unsigned int _n_points;
  /// Output control value
  Real & _output;
  /// Vector to store values
  std::vector<Real> & _values;
};
