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

class SinglePhaseFluidProperties;

/**
 * Compute internal energy given equation of state pressure and density
 */
class InternalEnergyAux : public AuxKernel
{
public:
  static InputParameters validParams();

  InternalEnergyAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const VariableValue & _pressure;
  const VariableValue & _rho;

  const SinglePhaseFluidProperties & _fp;
};
