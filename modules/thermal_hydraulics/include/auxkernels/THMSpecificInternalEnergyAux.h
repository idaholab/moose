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
