#pragma once

#include "IntegratedBC.h"

class CoupledGradBC : public IntegratedBC
{
public:
  static InputParameters validParams();

  CoupledGradBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

private:
  Real _sign;

  Real _coefficient;

  const Function & _func;

  const VariableGradient & _coupled_grad;
};
