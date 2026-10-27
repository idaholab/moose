//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "KokkosVectorKernelValue.h"

namespace Moose::Kokkos
{

/**
 * The base class for Kokkos vector time kernels whose test function can be factored out
 */
class VectorTimeKernelValue : public VectorKernelValue
{
public:
  static InputParameters validParams();

  VectorTimeKernelValue(const InputParameters & parameters);

protected:
  /// Time derivative of the current solution at quadrature points
  const VectorVariableValue _u_dot;
  /// Derivative of u_dot with respect to u
  const Scalar<const Real> _du_dot_du;
};

} // namespace Moose::Kokkos
