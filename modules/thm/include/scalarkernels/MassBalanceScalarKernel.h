#pragma once

#include "NodalScalarKernel.h"

class MassBalanceScalarKernel;

template <>
InputParameters validParams<MassBalanceScalarKernel>();

/**
 * Constraint to preserve mass balance
 */
class MassBalanceScalarKernel : public NodalScalarKernel
{
public:
  MassBalanceScalarKernel(const InputParameters & parameters);

  virtual void computeResidual();
  virtual void computeJacobian();

protected:
  unsigned int _rhoA_var_number;
  unsigned int _rhouA_var_number;
  const VariableValue & _rhouA;
  std::vector<Real> _normals;
};
