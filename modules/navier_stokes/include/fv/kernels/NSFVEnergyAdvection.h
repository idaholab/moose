//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NSFVMomentumAdvection.h"

#ifdef MOOSE_GLOBAL_AD_INDEXING

/**
 * An advection kernel that implements interpolation schemes specific to Navier-Stokes flow
 * physics
 */
class NSFVEnergyAdvection : public NSFVMomentumAdvection
{
public:
  static InputParameters validParams();
  NSFVEnergyAdvection(const InputParameters & params);
};

#endif
