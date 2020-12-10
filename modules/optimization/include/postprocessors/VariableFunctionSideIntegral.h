//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctionSideIntegral.h"

class Function;

/**
 * This postprocessor computes the inner product of a function and variable over a sideset
 */
class VariableFunctionSideIntegral : public FunctionSideIntegral
{
public:
  static InputParameters validParams();

  VariableFunctionSideIntegral(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// Holds the solution at current quadrature points
  const VariableValue & _u;
};
