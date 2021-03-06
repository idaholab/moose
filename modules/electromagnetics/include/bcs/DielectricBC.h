#pragma once

#include "DiffusionFluxBC.h"

class DielectricBC : public DiffusionFluxBC
{
public:
  static InputParameters validParams();

  DielectricBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  Real _eps_one;
  Real _eps_two;
};
