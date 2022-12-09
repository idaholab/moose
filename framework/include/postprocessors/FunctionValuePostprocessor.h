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

class Function;

/**
 * This postprocessor displays a single value which is supplied by a MooseFunction.
 * It is designed to prevent the need to create additional
 * postprocessors like DifferencePostprocessor.
 */
class FunctionValuePostprocessor : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  FunctionValuePostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual PostprocessorValue getValue() override;

protected:
  /// the function that will be evaluated and returned as pp value
  const Function & _function;
  /// a scale factor to scale the result of _function
  const Real & _scale_factor;
  /// true of space postprocessors have been provided
  bool _has_space_pp;
  /// a postprocessor that is passed to the time argument of _function (if provided)
  const PostprocessorValue * _time_pp;
  /// a vector of postprocessor values that are passed into the space argument of _function (if provided)
  std::vector<const PostprocessorValue *> _point;
};
