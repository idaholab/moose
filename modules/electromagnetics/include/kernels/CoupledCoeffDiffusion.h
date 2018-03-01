#ifndef COUPLEDCOEFFDIFFUSION_H
#define COUPLEDCOEFFDIFFUSION_H

#include "Diffusion.h"

class CoupledCoeffDiffusion;

template <>
InputParameters validParams<CoupledCoeffDiffusion>();

class CoupledCoeffDiffusion : public Kernel
{
public:
  CoupledCoeffDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

private:
  Real _coefficient;

  Function & _func;

  Real _sign;

  const VariableGradient & _coupled_grad;
};

#endif // COUPLEDCOEFFDIFFUSION_H
