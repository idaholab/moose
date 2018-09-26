//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECIFICINTERNALENERGYAUX_H
#define SPECIFICINTERNALENERGYAUX_H

#include "AuxKernel.h"

class SpecificInternalEnergyAux;

template <>
InputParameters validParams<SpecificInternalEnergyAux>();

/**
 * Computes specific internal energy
 */
class SpecificInternalEnergyAux : public AuxKernel
{
public:
  SpecificInternalEnergyAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const VariableValue & _rho;
  const VariableValue & _rho_u;
  const VariableValue & _rho_v;
  const VariableValue & _rho_w;
  const VariableValue & _rho_et;
};

#endif /* SPECIFICINTERNALENERGYAUX_H */
