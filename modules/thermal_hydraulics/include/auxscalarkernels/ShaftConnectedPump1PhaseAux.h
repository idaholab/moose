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

class ADShaftConnectedPump1PhaseUserObject;

/**
 * Computes various quantities for a ShaftConnectedPump1Phase.
 */
class ShaftConnectedPump1PhaseAux : public AuxScalarKernel
{
public:
  static InputParameters validParams();

  ShaftConnectedPump1PhaseAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Quantity type
  enum class Quantity
  {
    PUMP_HEAD,
    HYDRAULIC_TORQUE,
    FRICTION_TORQUE,
    MOMENT_OF_INERTIA
  };
  /// Which quantity to compute
  const Quantity _quantity;

  /// 1-phase shaft-connected pump user object
  const ADShaftConnectedPump1PhaseUserObject & _pump_uo;
};
