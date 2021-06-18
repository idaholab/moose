#pragma once

#include "AuxKernel.h"

class CoupledTimeDerivativeAux : public AuxKernel
{
public:
  static InputParameters validParams();

  CoupledTimeDerivativeAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  const VariableValue & _coupled_dt;
};
