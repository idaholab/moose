//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADKERNELSUPG_H
#define ADKERNELSUPG_H

#include "ADKernelStabilized.h"

#define usingTemplKernelSUPGMembers(type)                                                          \
  usingTemplKernelStabilizedMembers(type);                                                         \
  using ADKernelSUPGTempl<type, compute_stage>::_velocity;                                         \
  using ADKernelSUPGTempl<type, compute_stage>::_tau
#define usingKernelSUPGMembers usingTemplKernelSUPGMembers(Real)
#define usingVectorKernelSUPGMembers usingTemplKernelSUPGMembers(RealVectorValue)

template <typename, ComputeStage>
class ADKernelSUPGTempl;

template <ComputeStage compute_stage>
using ADKernelSUPG = ADKernelSUPGTempl<Real, compute_stage>;
template <ComputeStage compute_stage>
using ADVectorKernelSUPG = ADKernelSUPGTempl<RealVectorValue, compute_stage>;

declareADValidParams(ADKernelSUPG);
declareADValidParams(ADVectorKernelSUPG);

template <typename T, ComputeStage compute_stage>
class ADKernelSUPGTempl : public ADKernelStabilizedTempl<T, compute_stage>
{
public:
  ADKernelSUPGTempl(const InputParameters & parameters);

protected:
  ADRealVectorValue virtual computeQpStabilization() override;

  const ADMaterialProperty(Real) & _tau;
  const ADVectorVariableValue & _velocity;

  usingTemplKernelStabilizedMembers(T);
};

#endif /* ADKERNELSUPG_H */
