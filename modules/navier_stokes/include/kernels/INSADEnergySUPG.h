//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelSUPG.h"

/**
 * This class computes the residual and Jacobian contributions for
 * temperature/energy equation SUPG stabilization
 */
class INSADEnergySUPG : public ADKernelSUPG
{
public:
  static InputParameters validParams();

  INSADEnergySUPG(const InputParameters & parameters);

protected:
  ADReal precomputeQpStrongResidual() override;

  const ADMaterialProperty<Real> & _temperature_strong_residual;
};
