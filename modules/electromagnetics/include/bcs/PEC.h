#pragma once

#include "IntegratedBC.h"

class PEC : public IntegratedBC
{
public:
  static InputParameters validParams();

  PEC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  const VariableValue & _coupled_val_0;
  const VariableValue & _coupled_val_1;
  const VariableValue & _coupled_val_2;
};
