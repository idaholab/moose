#ifndef ROBINREFLECTIONBC_H
#define ROBINREFLECTIONBC_H

#include "IntegratedBC.h"

class RobinReflectionBC;

template <>
InputParameters validParams<RobinReflectionBC>();

class RobinReflectionBC : public IntegratedBC
{
public:
  RobinReflectionBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

private:
  const VariableValue & _field_real;

  const VariableValue & _field_imag;

  MooseEnum _component;

  Real _L;

  Function & _func_real;

  Function & _func_imag;

  Real _coeff_real;

  Real _coeff_imag;

  Real _sign;

  Real _RHS_coeff;
};

#endif // ROBINREFLECTIONBC_H
