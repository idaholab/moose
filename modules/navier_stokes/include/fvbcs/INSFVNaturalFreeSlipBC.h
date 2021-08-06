//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVSlipWallBC.h"

/**
 * A class for free slip boundary conditions for the velocity.
 */
class INSFVNaturalFreeSlipBC : public INSFVSlipWallBC
{
public:
  static InputParameters validParams();
  INSFVNaturalFreeSlipBC(const InputParameters & params);

  using INSFVSlipWallBC::gatherRCData;

  // A free slip wall boundary condition does not allow any outflow nor does it impose any viscous
  // stress so there is no data to contribute from this object
  void gatherRCData(const FaceInfo &) override {}
};
