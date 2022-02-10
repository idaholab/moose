//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SinglePhaseFluidPropertiesTestUtils.h"
#include "MooseTypes.h"

// Absolute tolerance to use for calculations that should only differ by the
// accumulation of roundoff
#define ABS_TOL_ROUNDOFF 1e-12

// Minimum perturbation to be used in FD Jacobian computations
#define PERTURBATION_MIN 1e-12

inline Real
computeFDPerturbation(const Real & value, const Real & relative_perturbation = REL_PERTURBATION)
{
  return std::max(relative_perturbation * std::abs(value), PERTURBATION_MIN);
}
