#pragma once

#include "Diffusion.h"

class FuncDiffusion : public Diffusion
{
public:
  static InputParameters validParams();

  FuncDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

private:
  const Function & _func;
};
