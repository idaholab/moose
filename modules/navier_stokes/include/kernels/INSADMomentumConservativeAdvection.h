//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelGrad.h"

/**
 * This class computes the momentum equation residual and Jacobian
 * contributions for the advective term of the incompressible Navier-Stokes momentum
 * equation.
 */
class INSADMomentumConservativeAdvection : public ADVectorKernelGrad
{
public:
  static InputParameters validParams();

  INSADMomentumConservativeAdvection(const InputParameters & parameters);

protected:
  virtual ADRealTensorValue precomputeQpResidual() override;

  /// The density
  const ADMaterialProperty<Real> & _rho;
};
