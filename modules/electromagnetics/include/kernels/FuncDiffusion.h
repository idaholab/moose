#ifndef FUNCDIFFUSION_H
#define FUNCDIFFUSION_H

#include "Diffusion.h"

class FuncDiffusion;

template <>
InputParameters validParams<FuncDiffusion>();

class FuncDiffusion : public Diffusion
{
public:
  FuncDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

private:

  const Function & _func;
};

#endif // FUNCDIFFUSION_H
