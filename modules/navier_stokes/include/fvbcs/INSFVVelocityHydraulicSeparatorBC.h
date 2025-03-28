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

class InputParameters;

/**
 * Class describing ah ydraulic separator for the velocity in the
 * Navier Stokes equations. There is no cross flow and this should also
 * ensure that the cell gradients are decoupled on the two sides of the boundary.
 */
class INSFVVelocityHydraulicSeparatorBC : public INSFVFluxBC
{
public:
  static InputParameters validParams();
  INSFVVelocityHydraulicSeparatorBC(const InputParameters & params);
};
