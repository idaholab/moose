//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "AuxKernel.h"

// Forward Declarations
class SinglePhaseFluidProperties;

/**
 * Auxiliary kernel for computing the Mach number assuming an ideal gas.
 */
class NSMachAux : public AuxKernel
{
public:
  static InputParameters validParams();

  NSMachAux(const InputParameters & parameters);

  virtual ~NSMachAux() {}

protected:
  virtual Real computeValue();

  const VariableValue & _u_vel;
  const VariableValue & _v_vel;
  const VariableValue & _w_vel;
  const VariableValue & _specific_volume;
  const VariableValue & _specific_internal_energy;

  // Fluid properties
  const SinglePhaseFluidProperties & _fp;
};
