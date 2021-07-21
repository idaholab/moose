//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"

// Finds the friction velocity using standard velocity wall functions formulation.
// It is used in WallFunctionWallShearStressAux, WallFunctionYPlusAux and
// INSFVWallFunctionBC.
ADReal findUStar(const Real mu, const Real rho, ADReal u, const Real dist);
