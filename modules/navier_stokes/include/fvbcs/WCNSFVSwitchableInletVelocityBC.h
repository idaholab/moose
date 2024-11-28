//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "WCNSFVInletVelocityBC.h"

/**
 * Dirichlet boundary conditions for the velocity, set from either a velocity postprocessor
 * The class is similar to WCNSFVInletVelocityBC but includes a switch
 * that allows us to switch on/off this boundary condition
 * or a mass flow rate divided by density and surface
 */
class WCNSFVSwitchableInletVelocityBC : public WCNSFVInletVelocityBC
{
public:
  static InputParameters validParams();
  WCNSFVSwitchableInletVelocityBC(const InputParameters & params);

protected:
  ADReal boundaryValue(const FaceInfo & fi, const Moose::StateArg & state) const override;

  /// Boolean switch to turn boundary condition on/off
  const bool & _switch_bc;

  /// Face limiter
  const Real & _face_limiter;
};
