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
#include "DerivativeMaterialInterface.h"

/**
 * ADSplitCHWResBase implements the residual for the chemical potential in the
 * split form of the Cahn-Hilliard equation in a general way that can be templated
 * to a scalar or tensor mobility.
 */
template <typename T>
class ADSplitCHWResBase : public ADKernelGrad
{
public:
  static InputParameters validParams();

  ADSplitCHWResBase(const InputParameters & parameters);

protected:
  virtual ADRealVectorValue precomputeQpResidual();

  const MaterialPropertyName _mob_name;
  const ADMaterialProperty<T> & _mob;
};

template <typename T>
ADSplitCHWResBase<T>::ADSplitCHWResBase(const InputParameters & parameters)
  : ADKernelGrad(parameters),
    _mob_name(getParam<MaterialPropertyName>("mob_name")),
    _mob(getADMaterialProperty<T>("mob_name"))
{
}

template <typename T>
ADRealVectorValue
ADSplitCHWResBase<T>::precomputeQpResidual()
{
  return _mob[_qp] * _grad_u[_qp];
}

template <typename T>
InputParameters
ADSplitCHWResBase<T>::validParams()
{
  InputParameters params = ADKernelGrad::validParams();
  params.addClassDescription(
      "Split formulation Cahn-Hilliard Kernel for the chemical potential variable");
  params.addParam<MaterialPropertyName>("mob_name", "mobtemp", "The mobility used with the kernel");
  return params;
}
