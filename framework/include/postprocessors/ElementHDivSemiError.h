//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementVectorL2Error.h"

/**
 * This postprocessor will print out the H(div)-seminorm of the difference
 * between the computed solution and the passed function.
 * ||u-f||_{H(div)} = sqrt( \int |div u - div f|^2 dx )
 */
class ElementHDivSemiError : public ElementVectorL2Error
{
public:
  static InputParameters validParams();

  ElementHDivSemiError(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// vector variable
  const VectorMooseVariable & _u_var;

  /// div of the vector variable
  const VectorVariableDivergence & _div_u;
};
