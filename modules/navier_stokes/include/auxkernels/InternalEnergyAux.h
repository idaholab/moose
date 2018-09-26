//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INTERNALENERGYAUX_H
#define INTERNALENERGYAUX_H

#include "AuxKernel.h"

class InternalEnergyAux;
class SinglePhaseFluidProperties;

template <>
InputParameters validParams<InternalEnergyAux>();

/**
 * Compute internal energy given equation of state pressure and density
 */
class InternalEnergyAux : public AuxKernel
{
public:
  InternalEnergyAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const VariableValue & _pressure;
  const VariableValue & _rho;

  const SinglePhaseFluidProperties & _fp;
};

#endif /* INTERNALENERGYAUX_H */
