#pragma once

#include "ADVectorTimeDerivative.h"

class CoeffADVectorTimeDerivative : public ADVectorTimeDerivative
{
public:
  static InputParameters validParams();

  CoeffADVectorTimeDerivative(const InputParameters & parameters);

protected:
  virtual ADRealVectorValue precomputeQpResidual() override;

  const Function & _coefficient;

  using KernelBase::_q_point;
};
