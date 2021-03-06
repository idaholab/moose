#pragma once

#include "Function.h"
#include "FunctionInterface.h"

class OneDFieldCoeff : public Function, public FunctionInterface
{
public:
  static InputParameters validParams();

  OneDFieldCoeff(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p) const override;

private:
  Real _theta;

  const Function & _eps_r;

  const Function & _inverse_mu_r;
};
