//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosIntegratedBC.h"

namespace Moose::Kokkos
{

/**
 * Name for a Kokkos integrated BC whose residual multiplies the test function value, defined
 * through the precomputeQpResidualValue() family of hooks of IntegratedBC
 */
using IntegratedBCValue = IntegratedBC;

} // namespace Moose::Kokkos
