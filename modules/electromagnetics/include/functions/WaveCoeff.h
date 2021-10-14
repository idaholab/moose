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
  /// Real component of the relative electric permittivity
  const Function & _eps_r_real;
  /// Imaginary component of the relative electric permittivity
  const Function & _eps_r_imag;

  /// Real component of the relative magnetic permeability
  const Function & _mu_r_real;
  /// Real component of the relative magnetic permeability
  const Function & _mu_r_imag;

  /// Real component of the wave number
  const Function & _k_real;
  /// Imaginery component of the wave number (also known as the attenuation constant)
  const Function & _k_imag;

  /// Signifies whether function output should be the real or imaginary component
  const MooseEnum _component;
};
