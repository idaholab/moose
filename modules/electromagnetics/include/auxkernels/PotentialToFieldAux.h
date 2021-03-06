#pragma once

#include "VariableGradientComponent.h"

class PotentialToFieldAux : public VariableGradientComponent
{
public:
  static InputParameters validParams();

  PotentialToFieldAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  Real _sign;
};
