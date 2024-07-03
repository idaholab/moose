//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADLaplacianSplit.h"

/**
 * Split with a variable that holds the Laplacian of the phase field with a constant prefactor.
 */
class ADPrefactorLaplacianSplit : public ADLaplacianSplit
{
public:
  static InputParameters validParams();

  ADPrefactorLaplacianSplit(const InputParameters & parameters);

protected:
  virtual ADRealGradient precomputeQpResidual() override;
  const Real _prefactor;
  const ADMaterialProperty<Real> & _rho_val;
};
