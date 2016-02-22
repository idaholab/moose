#ifndef SPECIFICINTERNALENERGYAUX_H
#define SPECIFICINTERNALENERGYAUX_H

#include "AuxKernel.h"

class SpecificInternalEnergyAux;

template<>
InputParameters validParams<SpecificInternalEnergyAux>();

/**
 * Computes specific internal energy
 */
class SpecificInternalEnergyAux : public AuxKernel
{
public:
  SpecificInternalEnergyAux(const InputParameters & parameters);
  virtual ~SpecificInternalEnergyAux();

protected:
  virtual Real computeValue();

  /// density, rho
  const VariableValue & _rho;
  /// momentum, rhou
  const VariableValue & _rhou;
  /// total energy, rhoE
  const VariableValue & _rhoE;
};

#endif /* SPECIFICINTERNALENERGYAUX_H */
