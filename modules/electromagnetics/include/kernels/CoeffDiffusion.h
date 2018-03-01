#ifndef COEFFDIFFUSION_H
#define COEFFDIFFUSION_H

#include "Diffusion.h"

class CoeffDiffusion;

template <>
InputParameters validParams<CoeffDiffusion>();

class CoeffDiffusion : public Diffusion
{
public:
  CoeffDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

private:
  Real _coefficient;

  Function & _func;
};

#endif // COEFFDIFFUSION_H
