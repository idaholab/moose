//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADALLENCAHNBASE_H
#define ADALLENCAHNBASE_H

#include "ADKernelValue.h"
#include "DerivativeMaterialPropertyNameInterface.h"

#define usingAllenCahnBaseMembers(T)                                                               \
  usingKernelValueMembers;                                                                         \
  using ADAllenCahnBase<compute_stage, T>::_prop_L;                                                \
  using ADAllenCahnBase<compute_stage, T>::computeDFDOP

// Forward declarations
template <ComputeStage compute_stage, typename T = void>
class ADAllenCahnBase;

declareADValidParams(ADAllenCahnBase);

/**
 * This is the Allen-Cahn equation base class that implements the bulk or
 * local energy term of the equation. It is templated on the type of the mobility,
 * which can be either a number (Real) or a tensor (RealValueTensor).
 * Note that the function computeDFDOP MUST be overridden in any kernel that inherits from
 * ADAllenCahnBase. This is the AD equivalent of ACBulk<>.
 */
template <ComputeStage compute_stage, typename T>
class ADAllenCahnBase : public ADKernelValue<compute_stage>,
                        public DerivativeMaterialPropertyNameInterface
{
public:
  ADAllenCahnBase(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual ADReal precomputeQpResidual();

  /// Compute the derivative of the bulk free energy w.r.t the order parameter
  virtual ADReal computeDFDOP() = 0;

  /// Mobility
  const ADMaterialProperty(T) & _prop_L;

  usingKernelValueMembers;
};

template <ComputeStage compute_stage, typename T>
ADAllenCahnBase<compute_stage, T>::ADAllenCahnBase(const InputParameters & parameters)
  : ADKernelValue<compute_stage>(parameters),
    DerivativeMaterialPropertyNameInterface(),
    _prop_L(adGetADMaterialProperty<T>("mob_name"))
{
}

template <ComputeStage compute_stage, typename T>
ADReal
ADAllenCahnBase<compute_stage, T>::precomputeQpResidual()
{
  return _prop_L[_qp] * computeDFDOP();
}

#endif // ADALLENCAHNBASE_H
