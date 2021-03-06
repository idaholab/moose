#pragma once

#include "DiffusionFluxBC.h"

class DielectricBC : public DiffusionFluxBC
{
public:
  static InputParameters validParams();

  DielectricBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  Real _eps_one;
  Real _eps_two;
};
