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
 * Hydraulic torque computed in the 1-phase shaft-connected pump
 */
class HydraulicTorqueAux : public AuxScalarKernel
{
public:
  HydraulicTorqueAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();
  /// 1-phase shaft-connected pump user object
  const ADShaftConnectedPump1PhaseUserObject & _pump_uo;

public:
  static InputParameters validParams();
};
