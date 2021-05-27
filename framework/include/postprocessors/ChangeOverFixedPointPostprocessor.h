//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

/**
 * Computes the change in a post-processor value, or the magnitude of its
 * relative change, over a time step or over the entire transient.
 */
class ChangeOverFixedPointPostprocessor : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  ChangeOverFixedPointPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual Real getValue() override;

private:
  /// option to compute change with respect to initial value instead of previous time value
  const bool _change_with_respect_to_initial;

  /// option to compute the magnitude of relative change instead of change
  const bool _compute_relative_change;

  /// option to take the absolute value of the change
  const bool _take_absolute_value;

  /// current post-processor value
  const PostprocessorValue & _pps_value;

  /// post-processor value at the previous fixed point iteration
  Real _pps_value_old;

  /// initial post-processor value
  Real & _pps_value_initial;

  /// the previous time step
  int _t_step_old;
};
