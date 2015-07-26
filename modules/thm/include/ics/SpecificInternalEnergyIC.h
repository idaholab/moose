#ifndef SPECIFICINTERNALENERGYIC_H
#define SPECIFICINTERNALENERGYIC_H

#include "InitialCondition.h"

class SpecificInternalEnergyIC;

template<>
InputParameters validParams<SpecificInternalEnergyIC>();

/**
 *
 */
class SpecificInternalEnergyIC : public InitialCondition
{
public:
  SpecificInternalEnergyIC(const InputParameters & parameters);
  virtual ~SpecificInternalEnergyIC();

  virtual Real value(const Point & p);

protected:
  VariableValue & _rho;
  VariableValue & _rhou;
  VariableValue & _rhoE;
};


#endif /* SPECIFICINTERNALENERGYIC_H */
