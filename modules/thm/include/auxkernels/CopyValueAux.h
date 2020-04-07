#pragma once

#include "AuxKernel.h"

/**
 *
 */
class CopyValueAux : public AuxKernel
{
public:
  CopyValueAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const VariableValue & _source_var;

public:
  static InputParameters validParams();
};
