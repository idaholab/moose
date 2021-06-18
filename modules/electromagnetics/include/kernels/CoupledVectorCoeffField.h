#pragma once

#include "VectorKernel.h"

class CoupledVectorCoeffField : public VectorKernel
{
public:
  static InputParameters validParams();

  CoupledVectorCoeffField(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  Real _coefficient;

  const Function & _func;

  const VectorVariableValue & _coupled_val;
};
