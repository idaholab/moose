#pragma once

#include "AuxKernel.h"

/**
 * Takes a boolean value and converts it into a Real value (0 for false, 1 for true)
 */
class BooleanValueTestAux : public AuxKernel
{
public:
  BooleanValueTestAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  const bool & _value;

public:
  static InputParameters validParams();
};
