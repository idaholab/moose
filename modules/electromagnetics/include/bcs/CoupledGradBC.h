#pragma once

#include "ADIntegratedBC.h"

class CoupledGradBC : public ADIntegratedBC
{
public:
  static InputParameters validParams();

  CoupledGradBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

private:
  Real _sign;

  Real _coefficient;

  const Function & _func;

  const ADVariableGradient & _coupled_grad;
};
