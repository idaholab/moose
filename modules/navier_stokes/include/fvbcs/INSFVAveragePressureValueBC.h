//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVBoundaryIntegralValueConstraint.h"
#include "INSFVFullyDevelopedFlowBC.h"

/**
 * A class for setting the value of the pressure at an outlet of the system.
 * It may not be used with a mean/pinned-pressure approach
 */
class INSFVAveragePressureValueBC : public FVBoundaryIntegralValueConstraint,
                                    public INSFVFullyDevelopedFlowBC
{
public:
  static InputParameters validParams();
  INSFVAveragePressureValueBC(const InputParameters & params);
};
