#pragma once

#include "VectorTimeKernel.h"

class VectorSecondTimeDerivative;

template <>
InputParameters validParams<VectorSecondTimeDerivative>();

/**
 *
 */
class VectorSecondTimeDerivative : public VectorTimeKernel
{
public:
  VectorSecondTimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  const VectorVariableValue & _u_dot_dot;
  const VariableValue & _du_dot_dot_du;
};
