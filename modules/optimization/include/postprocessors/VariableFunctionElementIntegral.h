//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementIntegralPostprocessor.h"

class VariableFunctionElementIntegral;

template <>
InputParameters validParams<VariableFunctionElementIntegral>();

/**
 * Integrates a function over elements
 */
class VariableFunctionElementIntegral : public ElementIntegralPostprocessor,
                                        public MooseVariableInterface<Real>
{
public:
  static InputParameters validParams();

  VariableFunctionElementIntegral(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// Function to integrate
  const Function & _function;

  /// Holds the solution at current quadrature points
  const VariableValue & _u;

  /// Holds the solution gradient at the current quadrature points
  const VariableGradient & _grad_u;

  const Real & _scale_factor;
};
