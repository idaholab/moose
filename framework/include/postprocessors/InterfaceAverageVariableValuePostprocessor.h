//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceIntegralVariableValuePostprocessor.h"

/**
 * This postprocessor computes a spatial average value of the specified variable value on the
 * interface. Different kind of averages may be obtaine either by specializing the computeQpIntegral
 * or adding a new type of interfaceValue to InterfaceValueTools class
 */
class InterfaceAverageVariableValuePostprocessor
  : public InterfaceIntegralVariableValuePostprocessor
{
public:
  static InputParameters validParams();

  InterfaceAverageVariableValuePostprocessor(const InputParameters & parameters);
  virtual Real getValue() override;
};
