//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MomentumFreeSlipBC.h"

/**
 * Boundary condition for free slip wall boundary specifically applied to privitive velocity
 * variables
 */
class INSFEMomentumFreeSlipBC : public MomentumFreeSlipBC
{
public:
  static InputParameters validParams();

  INSFEMomentumFreeSlipBC(const InputParameters & parameters);
};
