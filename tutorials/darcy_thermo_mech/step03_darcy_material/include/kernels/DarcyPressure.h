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
  virtual ADRealVectorValue precomputeQpResidual() override;

  // References to be set from Material system

  /// The permeability. Note that this is declared as a \p MaterialProperty. This means that if
  /// calculation of this property in the producing \p Material depends on non-linear variables, the
  /// derivative information will be lost here in the consumer and the non-linear solve will suffer
  const MaterialProperty<Real> & _permeability;

  /// The viscosity. This is declared as an \p ADMaterialProperty, meaning any derivative
  /// information coming from the producing \p Material will be preserved and the integrity of the
  /// non-linear solve will be likewise preserved
  const ADMaterialProperty(Real) & _viscosity;

  usingKernelGradMembers;
};
