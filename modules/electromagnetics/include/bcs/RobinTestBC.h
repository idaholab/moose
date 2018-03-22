#ifndef ROBINTESTBC_H
#define ROBINTESTBC_H

#include "IntegratedBC.h"

class RobinTestBC;

template <>
InputParameters validParams<RobinTestBC>();

class RobinTestBC : public IntegratedBC
{
public:
  RobinTestBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

private:
  const VariableValue & _coupled_val;

  MooseEnum _component;

  Real _L;

  Function & _func_real;

  Function & _func_imag;

  Real _coeff_real;

  Real _coeff_imag;
};

#endif // ROBINTESTBC_H
