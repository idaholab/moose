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
 * This class computes the Smagorinsky LES eddy viscosity residual and Jacobian
 * contributions for that term of the LES filtered incompressible Navier-Stokes momentum
 * equation.
 */
class INSADSmagorinskyEddyViscosity : public ADVectorKernelGrad
{
public:
  INSADSmagorinskyEddyViscosity(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  ADRealTensorValue precomputeQpResidual() override;

  /// This model calculates a kinematic viscosity, so rho must multiply this
  const ADMaterialProperty<Real> & _rho;

  /// Value of Smagorinsky constant (dimensionless). The theory predicts this to be 0.18.
  const Real _smagorinsky_constant;
};
