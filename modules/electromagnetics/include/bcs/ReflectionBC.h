#ifndef REFLECTIONBC_H
#define REFLECTIONBC_H

#include "IntegratedBC.h"

class ReflectionBC;

template <>
InputParameters validParams<ReflectionBC>();

class ReflectionBC : public IntegratedBC
{
public:
  ReflectionBC(const InputParameters & parameters);

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

  Real _RHS_coeff;
};

#endif // REFLECTIONBC_H
