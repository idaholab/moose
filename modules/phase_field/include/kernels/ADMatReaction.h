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
 * This kernel adds to the residual a contribution of \f$ -L*v \f$ where \f$ L \f$ is a material
 * property and \f$ v \f$ is a variable (nonlinear or coupled).
 */
class ADMatReaction : public ADKernel
{
public:
  static InputParameters validParams();

  ADMatReaction(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual();

  /**
   * Kernel variable (can be nonlinear or coupled variable)
   * (For constrained Allen-Cahn problems, v = lambda
   * where lambda is the Lagrange multiplier)
   */
  const ADVariableValue & _v;

  /// Reaction rate
  const ADMaterialProperty<Real> & _mob;
};
