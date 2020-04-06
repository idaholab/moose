#pragma once

#include "AuxKernel.h"

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

public:
  static InputParameters validParams();
};
