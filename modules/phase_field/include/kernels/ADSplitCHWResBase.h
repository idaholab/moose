//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADSPLITCHWRESBASE_H
#define ADSPLITCHWRESBASE_H

#include "ADKernelGrad.h"
#include "DerivativeMaterialInterface.h"

#define usingSplitCHWResBase                                                                       \
  usingKernelMembers;                                                                              \
  using ADSplitCHCRes<compute_stage>::_mob_name;                                                   \
  using ADSplitCHCRes<compute_stage>::_mob

// Forward declarations
template <ComputeStage compute_stage, typename T = void>
class ADSplitCHWResBase;

declareADValidParams(ADSplitCHWResBase);

/**
 * ADSplitCHWResBase implements the residual for the chemical potential in the
 * split form of the Cahn-Hilliard equation in a general way that can be templated
 * to a scalar or tensor mobility.
 */
template <ComputeStage compute_stage, typename T>
class ADSplitCHWResBase : public ADKernelGrad<compute_stage>
{
public:
  ADSplitCHWResBase(const InputParameters & parameters);

protected:
  virtual ADRealVectorValue precomputeQpResidual();

  const MaterialPropertyName _mob_name;
  const ADMaterialProperty(T) & _mob;

  usingKernelGradMembers;
};

template <ComputeStage compute_stage, typename T>
ADSplitCHWResBase<compute_stage, T>::ADSplitCHWResBase(const InputParameters & parameters)
  : ADKernelGrad<compute_stage>(parameters),
    _mob_name(adGetParam<MaterialPropertyName>("mob_name")),
    _mob(adGetADMaterialProperty<T>("mob_name"))
{
}

template <ComputeStage compute_stage, typename T>
ADRealVectorValue
ADSplitCHWResBase<compute_stage, T>::precomputeQpResidual()
{
  return _mob[_qp] * _grad_u[_qp];
}

#endif // ADSPLITCHWRESBASE_H
