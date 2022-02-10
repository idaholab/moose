//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxScalarKernel.h"

/**
 * Computes magnitude of velocity from the 1-phase volume junction variables
 */
class VolumeJunction1PhaseVelocityMagnitudeAux : public AuxScalarKernel
{
public:
  VolumeJunction1PhaseVelocityMagnitudeAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// rho*V of the junction
  const VariableValue & _rhoV;
  /// rho*u*V of the junction
  const VariableValue & _rhouV;
  /// rho*v*V of the junction
  const VariableValue & _rhovV;
  /// rho*w*V of the junction
  const VariableValue & _rhowV;

public:
  static InputParameters validParams();
};
