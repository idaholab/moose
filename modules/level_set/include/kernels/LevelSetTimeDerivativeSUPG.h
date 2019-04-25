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
#include "ADTimeKernelGrad.h"
#include "LevelSetVelocityInterface.h"

// Forward declarations
template <ComputeStage>
class LevelSetTimeDerivativeSUPG;

declareADValidParams(LevelSetTimeDerivativeSUPG);

/**
 * Applies SUPG stabilization to the time derivative.
 */
template <ComputeStage compute_stage>
class LevelSetTimeDerivativeSUPG : public LevelSetVelocityInterface<ADTimeKernelGrad<compute_stage>>
{
public:
  LevelSetTimeDerivativeSUPG(const InputParameters & parameters);

protected:
  virtual ADVectorResidual precomputeQpResidual() override;

  usingTimeKernelGradMembers;
  using LevelSetVelocityInterface<ADTimeKernelGrad<compute_stage>>::computeQpVelocity;
  using LevelSetVelocityInterface<ADTimeKernelGrad<compute_stage>>::_velocity;
};

