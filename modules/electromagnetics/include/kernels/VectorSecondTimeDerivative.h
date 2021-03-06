#pragma once

#include "VectorTimeKernel.h"

class VectorSecondTimeDerivative : public VectorTimeKernel
{
public:
  static InputParameters validParams();

  VectorSecondTimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  const VectorVariableValue & _u_dot_dot;
  const VariableValue & _du_dot_dot_du;

  const Function & _coeff;
};
