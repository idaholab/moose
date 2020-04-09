//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalBC.h"

// Forward Declarations
class IdealGasFluidProperties;

class NSThermalBC : public NodalBC
{
public:
  static InputParameters validParams();

  NSThermalBC(const InputParameters & parameters);

protected:
  // Computes the temperature based on ideal gas equation of state,
  // the total energy, and the velocity: T = e_i/c_v
  virtual Real computeQpResidual();

  unsigned int _rho_var;
  const VariableValue & _rho;

  Real _initial;
  Real _final;
  Real _duration;

  // Fluid properties
  const IdealGasFluidProperties & _fp;
};
