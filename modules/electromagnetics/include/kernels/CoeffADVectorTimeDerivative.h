#pragma once

#include "ADVectorTimeDerivative.h"

class CoeffADVectorTimeDerivative;

declareADValidParams(CoeffADVectorTimeDerivative);

class CoeffADVectorTimeDerivative : public ADVectorTimeDerivative
{
public:
  CoeffADVectorTimeDerivative(const InputParameters & parameters);

protected:
  virtual ADRealVectorValue precomputeQpResidual() override;

  const Function & _coefficient;

  using KernelBase::_q_point;
};
