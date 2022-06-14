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
 *  Kernel representing the contribution of the PDE term $m*u$, where $m$ is a
 *  material property coefficient/reaction rate and $u$ is a scalar variable, and
 *  whose Jacobian contribution is calculated using sutomatic differentiation.
 */
class ADMatReaction : public ADKernel
{
public:
  static InputParameters validParams();

  ADMatReaction(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

private:
  /// Material property coefficient / reaction rate
  const ADMaterialProperty<Real> & _mat_prop;
};
