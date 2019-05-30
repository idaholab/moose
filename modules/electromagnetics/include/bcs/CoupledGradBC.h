#ifndef COUPLEDGRADBC_H
#define COUPLEDGRADBC_H

#include "IntegratedBC.h"

class CoupledGradBC;

template <>
InputParameters validParams<CoupledGradBC>();

class CoupledGradBC : public IntegratedBC
{
public:
  CoupledGradBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

private:
  Real _sign;

  Real _coefficient;

  const Function & _func;

  const VariableGradient & _coupled_grad;
};

#endif // COUPLEDGRADBC_H
