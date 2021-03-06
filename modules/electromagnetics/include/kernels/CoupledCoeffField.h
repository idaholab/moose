#pragma once

#include "Kernel.h"

class CoupledCoeffField : public Kernel
{
public:
  static InputParameters validParams();

  CoupledCoeffField(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

private:
  Real _coefficient;

  const Function & _func;

  const VariableValue & _coupled_val;

  Real _sign;
};
