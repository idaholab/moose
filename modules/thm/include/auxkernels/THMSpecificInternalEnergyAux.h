#ifndef THMSPECIFICINTERNALENERGYAUX_H
#define THMSPECIFICINTERNALENERGYAUX_H

#include "AuxKernel.h"

class THMSpecificInternalEnergyAux;

template <>
InputParameters validParams<THMSpecificInternalEnergyAux>();

/**
 * Computes specific internal energy
 */
class THMSpecificInternalEnergyAux : public AuxKernel
{
public:
  THMSpecificInternalEnergyAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// density, rho
  const VariableValue & _rho;
  /// momentum, rhou
  const VariableValue & _rhou;
  /// total energy, rhoE
  const VariableValue & _rhoE;
};

#endif /* THMSPECIFICINTERNALENERGYAUX_H */
