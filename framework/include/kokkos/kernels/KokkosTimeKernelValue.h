//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "KokkosKernelValue.h"

namespace Moose::Kokkos
{

/**
 * The base class for Kokkos time kernels whose test function can be factored out
 */
class TimeKernelValue : public KernelValue
{
public:
  static InputParameters validParams();

  TimeKernelValue(const InputParameters & parameters);

protected:
  /// Time derivative of the current solution at quadrature points
  const VariableValue _u_dot;
  /// Derivative of u_dot with respect to u
  const Scalar<const Real> _du_dot_du;
};

} // namespace Moose::Kokkos
