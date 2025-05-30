//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

/**
 * Integrate a post-processor value over time using trapezoidal rule
 */
class TimeIntegratedPostprocessor : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  TimeIntegratedPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual Real getValue() const override;

protected:
  /// The total value of the variable
  Real _value;

  /// My old value
  const PostprocessorValue & _value_old;

  /// The current post-processor value
  const PostprocessorValue & _pps_value;

  /// The old post-processor value
  const PostprocessorValue & _pps_value_old;
};
