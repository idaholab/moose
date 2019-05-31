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

// Forward declarations
template <ComputeStage>
class LevelSetForcingFunctionSUPG;

declareADValidParams(LevelSetForcingFunctionSUPG);

/**
 * SUPG stabilization term for a forcing function.
 */
template <ComputeStage compute_stage>
class LevelSetForcingFunctionSUPG : public LevelSetVelocityInterface<ADKernelGrad<compute_stage>>
{
public:
  LevelSetForcingFunctionSUPG(const InputParameters & parameters);

protected:
  virtual ADVectorResidual precomputeQpResidual() override;

  /// Function value
  const Function & _function;

  usingKernelGradMembers;
  using LevelSetVelocityInterface<ADKernelGrad<compute_stage>>::computeQpVelocity;
  using LevelSetVelocityInterface<ADKernelGrad<compute_stage>>::_velocity;
  using LevelSetVelocityInterface<ADKernelGrad<compute_stage>>::_q_point;
};
