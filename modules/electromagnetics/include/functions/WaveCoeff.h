#pragma once

#include "Function.h"
#include "FunctionInterface.h"

/**
 *    Function for use as coefficient in standard-form Helmholtz wave equation applications.
 */
class WaveCoeff : public Function, public FunctionInterface
{
public:
  static InputParameters validParams();

  WaveCoeff(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p) const override;

protected:
  const Function & _eps_r_real;
  const Function & _eps_r_imag;

  const Function & _mu_r_real;
  const Function & _mu_r_imag;

  Real _k;

  MooseEnum _component;
};
