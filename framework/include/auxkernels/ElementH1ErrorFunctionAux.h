//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "ElementL2ErrorFunctionAux.h"

/**
 * A class for computing the element-wise H1 error (actually W^{1,p}
 * error, if you set the value of p to something other than 2.0) of
 * the difference between an exact solution (typically represented by
 * a ParsedFunction) and the specified solution variable.
 */
class ElementH1ErrorFunctionAux : public ElementL2ErrorFunctionAux
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   * @param name Object name
   * @param parameters Object input parameters
   */
  ElementH1ErrorFunctionAux(const InputParameters & parameters);

  /**
   * Overrides ElementLpNormAux since we want to raise to a power
   * in computeValue() instead.
   */
  virtual void compute() override;

protected:
  /**
   * Computes the error at the current qp.
   */
  virtual Real computeValue() override;

  /**
   * The gradient of the computed solution.
   */
  const VariableGradient & _grad_coupled_var;
};
