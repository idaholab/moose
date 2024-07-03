//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"

/**
 * Base class holding parameters for setting up NSFV actions
 */
class NSFVBase : public Action
{
public:
  static InputParameters validParams();
  static InputParameters commonNavierStokesFlowParams();
  static InputParameters commonMomentumEquationParams();
  static InputParameters commonMomentumBoundaryTypesParams();
  static InputParameters commonMomentumBoundaryFluxesParams();
  static InputParameters commonFluidEnergyEquationParams();
  static InputParameters commonScalarFieldAdvectionParams();
  static InputParameters commonTurbulenceParams();
};
