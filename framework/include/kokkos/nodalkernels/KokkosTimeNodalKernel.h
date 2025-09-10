//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosNodalKernel.h"

namespace Moose
{
namespace Kokkos
{

/**
 * The base class for Kokkos time-derivative nodal kernels
 */
class TimeNodalKernel : public NodalKernel
{
public:
  static InputParameters validParams();

  /**
   * Constructor
   */
  TimeNodalKernel(const InputParameters & parameters);

protected:
  /**
   * Time derivative of the current solution at nodes
   */
  const VariableNodalValue _u_dot;
  /**
   * Derivative of u_dot with respect to u
   */
  const Scalar<const Real> _du_dot_du;
};

} // namespace Kokkos
} // namespace Moose
