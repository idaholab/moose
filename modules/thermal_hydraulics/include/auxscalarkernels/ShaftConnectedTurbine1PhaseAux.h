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
#include "AuxScalarKernel.h"

class ADShaftConnectedTurbine1PhaseUserObject;

/**
 * Computes various quantities for a ShaftConnectedTurbine1Phase.
 */
template <typename T>
class ShaftConnectedTurbine1PhaseAuxTempl : public T
{
public:
  static InputParameters validParams();

  ShaftConnectedTurbine1PhaseAuxTempl(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Quantity type
  enum class Quantity
  {
    DELTA_P,
    FLOW_COEFFICIENT,
    DRIVING_TORQUE,
    FRICTION_TORQUE,
    MOMENT_OF_INERTIA,
    POWER
  };
  /// Which quantity to compute
  const Quantity _quantity;

  /// 1-phase shaft-connected turbine user object
  const ADShaftConnectedTurbine1PhaseUserObject & _turbine_uo;
};

typedef ShaftConnectedTurbine1PhaseAuxTempl<AuxKernel> ShaftConnectedTurbine1PhaseAux;
typedef ShaftConnectedTurbine1PhaseAuxTempl<AuxScalarKernel> ShaftConnectedTurbine1PhaseScalarAux;
