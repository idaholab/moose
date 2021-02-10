//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

// Forward Declarations

/**
 * Auxiliary kernel for computing the internal energy of the fluid.
 */
class NSInternalEnergyAux : public AuxKernel
{
public:
  static InputParameters validParams();

  NSInternalEnergyAux(const InputParameters & parameters);

  virtual ~NSInternalEnergyAux() {}

protected:
  virtual Real computeValue();

  const VariableValue & _rho;
  const VariableValue & _u_vel;
  const VariableValue & _v_vel;
  const VariableValue & _w_vel;
  const VariableValue & _rho_et;
};
