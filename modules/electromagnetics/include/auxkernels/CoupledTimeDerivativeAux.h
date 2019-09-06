#pragma once

#include "AuxKernel.h"

class CoupledTimeDerivativeAux;

template <>
InputParameters validParams<CoupledTimeDerivativeAux>();

class CoupledTimeDerivativeAux : public AuxKernel
{
public:
  CoupledTimeDerivativeAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  const VariableValue & _coupled_dt;
};
