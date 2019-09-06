#pragma once

#include "Diffusion.h"

class CoupledFuncDiffusion;

template <>
InputParameters validParams<CoupledFuncDiffusion>();

class CoupledFuncDiffusion : public Kernel
{
public:
  CoupledFuncDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

private:

  const Function & _func;

  Real _sign;

  const VariableGradient & _coupled_grad;
};
