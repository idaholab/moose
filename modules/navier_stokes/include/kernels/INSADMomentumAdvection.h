//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelValue.h"

/**
 * This class computes the momentum equation residual and Jacobian
 * contributions for the advective term of the incompressible Navier-Stokes momentum
 * equation.
 */
class INSADMomentumAdvection : public ADVectorKernelValue
{
public:
  static InputParameters validParams();

  INSADMomentumAdvection(const InputParameters & parameters);

protected:
  virtual ADRealVectorValue precomputeQpResidual() override;

  const ADMaterialProperty<RealVectorValue> & _advective_strong_residual;
};
