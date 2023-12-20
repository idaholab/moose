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

class Function;

/**
 * This postprocessor will print out the L2-seminorm of the difference
 * between the computed solution and the passed function.
 * ||u-f||_{L2} = sqrt( \int |u - f|^2 dx )
 */
class ElementVectorL2Error : public ElementIntegralPostprocessor
{
public:
  static InputParameters validParams();

  ElementVectorL2Error(const InputParameters & parameters);

  virtual Real getValue() const override;

protected:
  virtual Real computeQpIntegral() override;

  /// vector or component-wise analytical solution to compare against
  const Function & _func;
  const Function & _funcx;
  const Function & _funcy;
  const Function & _funcz;

  /// vector or component-wise variable values
  const VectorVariableValue & _u;
  const VariableValue & _ux;
  const VariableValue & _uy;
  const VariableValue & _uz;

  /// whether the user chose to use a vector or component-wise solution/variable
  const bool _has_vector_function;
  const bool _has_scalar_function;
  const bool _has_vector_variable;
  const bool _has_scalar_variable;
};
