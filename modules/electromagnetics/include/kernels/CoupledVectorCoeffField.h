#pragma once

#include "VectorKernel.h"

class CoupledVectorCoeffField;

template <>
InputParameters validParams<CoupledVectorCoeffField>();

class CoupledVectorCoeffField : public VectorKernel
{
public:
  CoupledVectorCoeffField(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  Real _coefficient;

  const Function & _func;

  const VectorVariableValue & _coupled_val;
};
