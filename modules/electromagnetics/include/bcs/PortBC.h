#ifndef PORTBC_H
#define PORTBC_H

#include "IntegratedBC.h"

class PortBC;

template <>
InputParameters validParams<PortBC>();

class PortBC : public IntegratedBC
{
public:
  PortBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

private:
  const VariableValue & _field_real;

  const VariableValue & _field_imag;

  MooseEnum _component;

  Function & _func_real;

  Function & _func_imag;

  Function & _profile_func_real;

  Function & _profile_func_imag;

  Real _coeff_real;

  Real _coeff_imag;

  Real _sign;
};

#endif // PORTBC_H
