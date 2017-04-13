/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef STAGNATIONPRESSUREAUX_H
#define STAGNATIONPRESSUREAUX_H

#include "AuxKernel.h"

class StagnationPressureAux;
class SinglePhaseFluidProperties;

template <>
InputParameters validParams<StagnationPressureAux>();

/**
 * Compute stagnation pressure from specific volume, specific internal energy, and velocity.
 */
class StagnationPressureAux : public AuxKernel
{
public:
  StagnationPressureAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const VariableValue & _specific_volume;
  const VariableValue & _specific_internal_energy;
  const VariableValue & _velocity;

  const SinglePhaseFluidProperties & _fp;
};

#endif /* STAGNATIONPRESSUREAUX_H */
