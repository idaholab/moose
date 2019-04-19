//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef LEVELSETADVECTION_H
#define LEVELSETADVECTION_H

// MOOSE includes
#include "ADKernelValue.h"
#include "LevelSetVelocityInterface.h"

// Forward declarations
template <ComputeStage>
class LevelSetAdvection;

declareADValidParams(LevelSetAdvection);

/**
 * Advection Kernel for the levelset equation.
 *
 * \psi_i \vec{v} \nabla u,
 * where \vec{v} is the interface velocity that is a set of
 * coupled variables.
 */
template <ComputeStage compute_stage>
class LevelSetAdvection : public LevelSetVelocityInterface<ADKernelValue<compute_stage>>
{
public:
  LevelSetAdvection(const InputParameters & parameters);

protected:
  virtual ADResidual precomputeQpResidual() override;

  usingKernelValueMembers;
  using LevelSetVelocityInterface<ADKernelValue<compute_stage>>::computeQpVelocity;
  using LevelSetVelocityInterface<ADKernelValue<compute_stage>>::_velocity;
};

#endif // LEVELSETADVECTION_H
