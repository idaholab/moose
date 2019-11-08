//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Including the "ADDiffusion" Kernel so it can be extended
#include "ADDiffusion.h"

/**
 * Computes the residual contribution: K / mu * grad_u * grad_phi.
 */
template <ComputeStage compute_stage>
class DarcyPressure : public ADDiffusion<compute_stage>
{
public:
  static InputParameters validParams();

  DarcyPressure(const InputParameters & parameters);

protected:
  /// ADKernelGrad objects must override precomputeQpResidual
  virtual ADRealVectorValue precomputeQpResidual() override;

  // References to be set from Material system
  const MaterialProperty<Real> & _permeability;
  const MaterialProperty<Real> & _viscosity;

  usingKernelGradMembers;
};
