//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CoupledForce.h"

/**
 *  Kernel representing the contribution of the PDE term $mv$, where $m$ is a material property
 *  coefficient, $v$ is a coupled scalar variable, and Jacobian derivatives are calculated
 *  using automatic differentiation.
 */
class ADMatCoupledForce : public ADCoupledForce
{
public:
  static InputParameters validParams();

  ADMatCoupledForce(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

private:
  /// Material property coefficient for coupled source term
  const ADMaterialProperty<Real> & _mat_prop;
};
