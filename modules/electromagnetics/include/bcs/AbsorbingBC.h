#pragma once

#include "IntegratedBC.h"

class AbsorbingBC;

template <>
InputParameters validParams<AbsorbingBC>();

class AbsorbingBC : public IntegratedBC
{
public:
  AbsorbingBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

private:
  const VariableValue & _field_real;

  const VariableValue & _field_imag;

  MooseEnum _component;

  const Function & _func_real;

  const Function & _func_imag;

  Real _coeff_real;

  Real _coeff_imag;

  Real _sign;
};
