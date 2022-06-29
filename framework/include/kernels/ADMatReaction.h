//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"

/**
 *  Kernel representing the contribution of the PDE term $-L*v$, where $L$ is a
 *  reaction rate coefficient material property, $v$ is a scalar variable (nonlinear or coupled
 * auxvariable), and whose Jacobian contribution is calculated using automatic differentiation.
 */
class ADMatReaction : public ADKernel
{
public:
  static InputParameters validParams();

  ADMatReaction(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

private:
  /**
   * Kernel variable (can be nonlinear or coupled variable)
   * (For constrained Allen-Cahn problems such as those in
   * phase field, v = lambda where lambda is the Lagrange
   * multiplier)
   */
  const ADVariableValue & _v;

  /// Reaction rate material property
  const ADMaterialProperty<Real> & _reaction_rate;
};
