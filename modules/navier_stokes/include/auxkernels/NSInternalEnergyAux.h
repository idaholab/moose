//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NSINTERNALENERGYAUX_H
#define NSINTERNALENERGYAUX_H

#include "AuxKernel.h"

// Forward Declarations
class NSInternalEnergyAux;

template <>
InputParameters validParams<NSInternalEnergyAux>();

/**
 * Auxiliary kernel for computing the internal energy of the fluid.
 */
class NSInternalEnergyAux : public AuxKernel
{
public:
  NSInternalEnergyAux(const InputParameters & parameters);

  virtual ~NSInternalEnergyAux() {}

protected:
  virtual Real computeValue();

  const VariableValue & _rho;
  const VariableValue & _u_vel;
  const VariableValue & _v_vel;
  const VariableValue & _w_vel;
  const VariableValue & _rhoE;
};

#endif
