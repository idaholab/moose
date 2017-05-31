#ifndef SPECIFICINTERNALENERGYIC_H
#define SPECIFICINTERNALENERGYIC_H

#include "InitialCondition.h"

class SpecificInternalEnergyIC;

template <>
InputParameters validParams<SpecificInternalEnergyIC>();

/**
 *
 */
class SpecificInternalEnergyIC : public InitialCondition
{
public:
  SpecificInternalEnergyIC(const InputParameters & parameters);

  virtual Real value(const Point & p);

protected:
  const VariableValue & _rho;
  const VariableValue & _rhou;
  const VariableValue & _rhoE;
};

#endif /* SPECIFICINTERNALENERGYIC_H */
