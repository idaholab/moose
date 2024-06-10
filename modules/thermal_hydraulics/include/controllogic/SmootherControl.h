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
 * This block represents a controller that uses a moving average to make the input smoother.
 */
class SmootherControl : public THMControl
{
public:
  SmootherControl(const InputParameters & parameters);

  virtual void execute();
  static InputParameters validParams();

protected:
  /// Input data
  const Real & _input;
  /// Maximum number of data points to be used in the average calculation
  const unsigned int _n_points;
  /// Output computed by the Smooth control
  Real & _output;
  /// Vector to store values
  std::vector<Real> _values;
};
