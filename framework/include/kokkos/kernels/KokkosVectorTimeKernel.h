//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosVectorKernel.h"

namespace Moose::Kokkos
{

/**
 * The base class for Kokkos vector time-derivative kernels
 */
class VectorTimeKernel : public VectorKernel
{
public:
  static InputParameters validParams();

  /**
   * Constructor
   */
  VectorTimeKernel(const InputParameters & parameters);

protected:
  /**
   * Time derivative of the current solution at quadrature points
   */
  const VectorVariableValue _u_dot;
  /**
   * Derivative of u_dot with respect to u
   */
  const Scalar<const Real> _du_dot_du;
};

} // namespace Moose::Kokkos
