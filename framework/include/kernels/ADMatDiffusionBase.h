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

#define usingMatDiffusionBaseMembers(T)                                                            \
  usingKernelGradMembers;                                                                          \
  using ADMatDiffusionBase<compute_stage, T>::_D;                                                  \
  using ADMatDiffusionBase<compute_stage, T>::_grad_conc

// Forward declarations
template <ComputeStage compute_stage, typename T = void>
class ADMatDiffusionBase;

declareADValidParams(ADMatDiffusionBase);

/**
 * This class template implements a diffusion kernel with a mobility that can vary
 * spatially and can depend on variables in the simulation. Two classes are derived from
 * this template, MatDiffusion for isotropic diffusion and MatAnisoDiffusion for
 * anisotropic diffusion.
 *
 * \tparam T Type of the diffusion coefficient parameter. This can be Real for
 *           isotropic diffusion or RealTensorValue for the general anisotropic case.
 */
template <ComputeStage compute_stage, typename T>
class ADMatDiffusionBase : public ADKernelGrad<compute_stage>
{
public:
  ADMatDiffusionBase(const InputParameters & parameters);

protected:
  virtual ADRealVectorValue precomputeQpResidual() override;

  /// diffusion coefficient
  const ADMaterialProperty(T) & _D;

  /// Gradient of the concentration
  const ADVariableGradient & _grad_conc;

  usingKernelGradMembers;
};

template <ComputeStage compute_stage, typename T>
ADMatDiffusionBase<compute_stage, T>::ADMatDiffusionBase(const InputParameters & parameters)
  : ADKernelGrad<compute_stage>(parameters),
    _D(adGetADMaterialProperty<T>("D_name")),
    _grad_conc(isCoupled("conc") ? adCoupledGradient("conc") : _grad_u)
{
}

template <ComputeStage compute_stage, typename T>
ADVectorResidual
ADMatDiffusionBase<compute_stage, T>::precomputeQpResidual()
{
  return _D[_qp] * _grad_conc[_qp];
}
