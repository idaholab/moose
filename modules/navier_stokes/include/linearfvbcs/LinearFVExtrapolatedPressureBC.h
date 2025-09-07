//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVAdvectionDiffusionExtrapolatedBC.h"

/**
 * Class implementing an extrapolated boundary condition for linear finite
 * volume pressure variables.
 */
class LinearFVExtrapolatedPressureBC : public LinearFVAdvectionDiffusionExtrapolatedBC
{
public:
  static InputParameters validParams();

  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  LinearFVExtrapolatedPressureBC(const InputParameters & parameters);
};
