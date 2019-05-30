#ifndef WAVECOEFF_H
#define WAVECOEFF_H

#include "Function.h"
#include "FunctionInterface.h"

class WaveCoeff;

template <>
InputParameters validParams<WaveCoeff>();

/**
 *    Function for use as coefficient in standard-form Helmholtz wave equation applications.
 */
class WaveCoeff : public Function, public FunctionInterface
{
public:
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

#endif // WAVECOEFF_H
