#ifndef POTENTIALTOFIELDAUX_H
#define POTENTIALTOFIELDAUX_H

#include "VariableGradientComponent.h"

class PotentialToFieldAux;

template <>
InputParameters validParams<PotentialToFieldAux>();

/**
 *
 */
class PotentialToFieldAux : public VariableGradientComponent
{
public:
  PotentialToFieldAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  Real _sign;
};

#endif // POTENTIALTOFIELDAUX_H
