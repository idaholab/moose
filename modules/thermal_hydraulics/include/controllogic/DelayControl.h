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
#include "LinearInterpolation.h"

/**
 * Time delay control
 *
 * This component delays the input value by a time period 'tau'.
 *
 * Internally, we keep nodes (time, value) and use linear interpolation to reconstruct the
 * solution in the past.  We only keep as many nodes that we cover the time window given
 * by the time period 'tau'.
 */
class DelayControl : public THMControl
{
public:
  DelayControl(const InputParameters & parameters);

  virtual void execute();

protected:
  /**
   * Add a node to linear interpolation
   */
  void addFnPoint(const Real & t, const Real & val);

  /**
   * Linear interpolation
   *
   * @param t time when we sample the function
   */
  Real sampleFunction(const Real & t) const;

  /// Initial value
  const Real & _initial_value;
  /// Input data
  const Real & _input;
  /// Time period
  Real _tau;
  /// Output value
  Real & _value;
  /// Times when 'input' was sampled
  std::deque<Real> & _input_time;
  /// Values of 'input' corresponding to _input_time
  std::deque<Real> & _input_vals;

public:
  static InputParameters validParams();
};
