//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVFluxBC.h"
#include "INSFVHydraulicSeparatorInterface.h"

class InputParameters;

/**
 * Class describing a hydraulic separator for the velocity in the
 * Navier Stokes equations. There is no cross flow and this should also
 * ensure that the cell gradients are decoupled on the two sides of the boundary.
 */
class INSFVVelocityHydraulicSeparatorBC : public INSFVFluxBC,
                                          public INSFVHydraulicSeparatorInterface
{
public:
  static InputParameters validParams();
  INSFVVelocityHydraulicSeparatorBC(const InputParameters & params);

  using INSFVFluxBC::gatherRCData;
  // The flow separator does not allow any outflow nor does it impose any viscous
  // stress so there is no data to contribute from this object
  void gatherRCData(const FaceInfo &) override {}
};
