#pragma once

#include "IntegratedBC.h"

class RobinBC : public IntegratedBC
{
public:
  static InputParameters validParams();

  RobinBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

private:
  const VariableValue & _field_real;

  const VariableValue & _field_imag;

  const MooseEnum _component;

  const Function & _func_real;

  const Function & _func_imag;

  const Function & _profile_func_real;

  const Function & _profile_func_imag;

  Real _coeff_real;

  Real _coeff_imag;

  Real _sign;

  const MooseEnum _mode;
};
