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

// Forward declare the class being created and the validParams function
template <ComputeStage>
class DarcyPressure;

declareADValidParams(DarcyPressure);

/**
 * Computes the residual contribution: K / mu * grad_u * grad_phi.
 *
 * We are inheriting from ADDiffusion instead of from ADKernel because
 * the grad_u * grad_phi is already coded and all that is
 * needed is to specialize that calculation by multiplying by K / mu.
 */
template <ComputeStage compute_stage>
class DarcyPressure : public ADDiffusion<compute_stage>
{
public:
  DarcyPressure(const InputParameters & parameters);

protected:
  /// ADKernelGrad objects must override precomputeQpResidual
  virtual ADVectorResidual precomputeQpResidual() override;

  /// References to be set from Material system
  const MaterialProperty<Real> & _permeability;
  const ADMaterialProperty(Real) & _viscosity;

  usingKernelGradMembers;
};
