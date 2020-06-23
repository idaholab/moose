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

class FunctionValuePostprocessor;
class Function;

template <>
InputParameters validParams<FunctionValuePostprocessor>();

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
  const Function & _function;
  const Point & _point;
  const Real & _scale_factor;
};
