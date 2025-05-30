//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
 * Computes various quantities for a VolumeJunction1Phase.
 */
class VolumeJunction1PhaseAux : public AuxKernel
{
public:
  static InputParameters validParams();

  VolumeJunction1PhaseAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Quantity type
  enum class Quantity
  {
    PRESSURE,
    TEMPERATURE,
    SPEED
  };
  /// Which quantity to compute
  const Quantity _quantity;

  /// Volume of the junction
  const Real & _volume;
  /// rho*V of the junction
  const VariableValue & _rhoV;
  /// rho*u*V of the junction
  const VariableValue & _rhouV;
  /// rho*v*V of the junction
  const VariableValue & _rhovV;
  /// rho*w*V of the junction
  const VariableValue & _rhowV;
  /// rho*E*V of the junction
  const VariableValue & _rhoEV;

  /// Single-phase fluid properties user object
  const SinglePhaseFluidProperties & _fp;
};
