//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "ADKernelGrad.h"
#include "LevelSetVelocityInterface.h"

/**
 * SUPG stabilization for the advection portion of the level set equation.
 */
class LevelSetAdvectionSUPG : public LevelSetVelocityInterface<ADKernelGrad>
{
public:
  static InputParameters validParams();

  LevelSetAdvectionSUPG(const InputParameters & parameters);

protected:
  virtual ADRealVectorValue precomputeQpResidual() override;

  using LevelSetVelocityInterface<ADKernelGrad>::computeQpVelocity;
  using LevelSetVelocityInterface<ADKernelGrad>::_velocity;
};
