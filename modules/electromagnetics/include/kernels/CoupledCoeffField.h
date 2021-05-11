#pragma once

#include "ADKernel.h"

class CoupledCoeffField : public ADKernel
{
public:
  static InputParameters validParams();

  CoupledCoeffField(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

private:
  Real _coefficient;

  const Function & _func;

  const ADVariableValue & _coupled_val;

  Real _sign;
};
