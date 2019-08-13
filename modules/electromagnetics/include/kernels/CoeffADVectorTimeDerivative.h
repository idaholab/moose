#pragma once

#include "ADVectorTimeDerivative.h"

template <ComputeStage>
class CoeffADVectorTimeDerivative;

declareADValidParams(CoeffADVectorTimeDerivative);

template <ComputeStage compute_stage>
class CoeffADVectorTimeDerivative : public ADVectorTimeDerivative<compute_stage>
{
public:
  CoeffADVectorTimeDerivative(const InputParameters & parameters);

protected:
  virtual ADRealVectorValue precomputeQpResidual() override;

  const Function & _coefficient;

  usingVectorTimeKernelValueMembers;
  using KernelBase::_q_point;
};
