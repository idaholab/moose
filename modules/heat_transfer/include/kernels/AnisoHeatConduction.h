//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "DerivativeMaterialInterface.h"
#include "Kernel.h"

/**
 * This kernel implements the Laplacian operator
 * multiplied by a 2nd order tensor giving
 * anisotropic (direction specific) HeatConduction:
 * $\overline K \cdot \nabla u \cdot \nabla \phi_i$
 */
class AnisoHeatConduction : public DerivativeMaterialInterface<Kernel>
{
public:
  static InputParameters validParams();

  AnisoHeatConduction(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  virtual Real computeQpJacobian() override;

  const MaterialProperty<RankTwoTensor> & _k;
  const MaterialProperty<RankTwoTensor> & _dk_dT;
};
