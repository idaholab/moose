//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVOutletPressureBC.h"

/**
 * A class for setting the value of the pressure at an outlet of the system.
 * The class is similar to INSFVOutletPressureBC but includes a switch
 * that allows us to switch on/off this boundary condition
 * It may not be used with a mean/pinned-pressure approach
 */
class INSFVSwitchableOutletPressureBC : public INSFVOutletPressureBCTempl<INSFVFlowBC>
{
public:
  static InputParameters validParams();
  INSFVSwitchableOutletPressureBC(const InputParameters & params);

  ADReal boundaryValue(const FaceInfo & /* fi */,
                       const Moose::StateArg & /* state */) const override;

private:
  /// Boolean switch to turn boundary condition on/off
  const bool & _switch_bc;

  /// Face limiter
  const Real & _face_limiter;
};
