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
 * Kernel 2 of 2 for interfacial energy anisotropy in the Allen-Cahn equation as
 * implemented in R. Kobayashi, Physica D, 63, 410-423 (1993).
 * doi:10.1016/0167-2789(93)90120-P
 * This kernel implements the third term on the right side of eq. (3) of the paper.
 */
class ADACInterfaceKobayashi2 : public ADKernelGrad
{
public:
  static InputParameters validParams();

  ADACInterfaceKobayashi2(const InputParameters & parameters);

protected:
  ADRealGradient precomputeQpResidual() override;

  /// Mobility
  const ADMaterialProperty<Real> & _mob;

  /// Interfacial parameter
  const ADMaterialProperty<Real> & _eps;
};
