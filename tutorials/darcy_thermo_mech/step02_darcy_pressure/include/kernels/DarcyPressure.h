//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Including the "ADKernel" Kernel here so we can extend it
#include "ADKernel.h"

/**
 * Computes the residual contribution: K / mu * grad_u * grad_phi.
 *
 * We are inheriting from ADKernel instead of from ADKernel because
 * the grad_u * grad_phi is already coded and all that is
 * needed is to specialize that calculation by multiplying by K / mu.
 */
template <ComputeStage compute_stage>
class DarcyPressure : public ADKernel<compute_stage>
{
public:
  static InputParameters validParams();

  DarcyPressure(const InputParameters & parameters);

protected:
  /// ADKernelGrad objects must override precomputeQpResidual
  virtual ADReal computeQpResidual() override;

  /// References to be set from input file
  const Real & _permeability;
  const Real & _viscosity;

  usingKernelMembers;
};
