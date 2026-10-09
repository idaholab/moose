//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosADKernel.h"

namespace Moose::Kokkos
{

/**
 * The base class for Kokkos time-derivative kernels whose Jacobians are computed with pointwise
 * automatic differentiation. It tags the residual and Jacobian as time contributions; the derived
 * class reads the time derivative it needs through adDot() or adCoupledDot().
 */
class ADTimeKernel : public ADKernel
{
public:
  static InputParameters validParams();

  /**
   * Constructor
   */
  ADTimeKernel(const InputParameters & parameters);
};

} // namespace Moose::Kokkos
