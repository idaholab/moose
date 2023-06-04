/*
AuxKernel of Passing Variable
*/

#pragma once

#include "AuxKernel.h"

class CopyValueAux : public AuxKernel
{
public:
  static InputParameters validParams();
  CopyValueAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  const VariableValue & _coupled_val;
};
