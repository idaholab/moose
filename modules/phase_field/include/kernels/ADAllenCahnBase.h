//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelValue.h"
#include "DerivativeMaterialPropertyNameInterface.h"

/**
 * This is the Allen-Cahn equation base class that implements the bulk or
 * local energy term of the equation. It is templated on the type of the mobility,
 * which can be either a number (Real) or a tensor (RealValueTensor).
 * Note that the function computeDFDOP MUST be overridden in any kernel that inherits from
 * ADAllenCahnBase. This is the AD equivalent of ACBulk<>.
 */
template <typename T>
class ADAllenCahnBase : public ADKernelValue, public DerivativeMaterialPropertyNameInterface
{
public:
  ADAllenCahnBase(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual ADReal precomputeQpResidual();

  /// Compute the derivative of the bulk free energy w.r.t the order parameter
  virtual ADReal computeDFDOP() = 0;

  /// Mobility
  const ADMaterialProperty<T> & _prop_L;
};

template <typename T>
InputParameters
ADAllenCahnBase<T>::validParams()
{
  InputParameters params = ADKernelValue::validParams();
  params.addClassDescription(
      "Allen-Cahn bulk contribution Kernel with forward mode automatic differentiation");
  params.addParam<MaterialPropertyName>("mob_name", "L", "The mobility used with the kernel");
  return params;
}

template <typename T>
ADAllenCahnBase<T>::ADAllenCahnBase(const InputParameters & parameters)
  : ADKernelValue(parameters),
    DerivativeMaterialPropertyNameInterface(),
    _prop_L(getADMaterialProperty<T>("mob_name"))
{
}

template <typename T>
ADReal
ADAllenCahnBase<T>::precomputeQpResidual()
{
  return _prop_L[_qp] * computeDFDOP();
}
