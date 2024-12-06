//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ChainControl.h"

/**
 * Computes a moving average of the input control with a user-specified
 * number of points to average.
 **/
class SmootherChainControl : public ChainControl
{
public:
  static InputParameters validParams();

  SmootherChainControl(const InputParameters & parameters);

  virtual void execute() override;

protected:
  /// Performs the main execution
  void executeInner();

  /// Input data
  const Real & _input;
  /// The number of points to use in the moving average
  const unsigned int _n_points;
  /// Output control value
  Real & _output;
  /// Vector to store values
  std::vector<Real> & _values;
  /// Previous time for which value was cached
  Real & _previous_time;
};
