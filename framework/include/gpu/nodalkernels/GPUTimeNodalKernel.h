//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUNodalKernel.h"

namespace Moose
{
namespace Kokkos
{

/**
 * The base class for Kokkos time-derivative nodal kernels
 */
template <typename Derived>
class TimeNodalKernel : public NodalKernel<Derived>
{
  usingKokkosNodalKernelMembers(Derived);

public:
  static InputParameters validParams()
  {
    InputParameters params = NodalKernel<Derived>::validParams();

    params.set<MultiMooseEnum>("vector_tags") = "time";
    params.set<MultiMooseEnum>("matrix_tags") = "system time";

    return params;
  }

  /**
   * Constructor
   */
  TimeNodalKernel(const InputParameters & parameters)
    : NodalKernel<Derived>(parameters),
      _u_dot(kokkosSystems(), _var, Moose::SOLUTION_DOT_TAG),
      _du_dot_du(_var.sys().duDotDu(_var.number()))
  {
  }

protected:
  /**
   * Time derivative of the current solution at nodes
   */
  VariableNodalValue _u_dot;
  /**
   * Derivative of u_dot with respect to u
   */
  Scalar<const Real> _du_dot_du;
};

} // namespace Kokkos
} // namespace Moose

#define usingKokkosTimeNodalKernelMembers(T)                                                       \
  usingKokkosNodalKernelMembers(T);                                                                \
                                                                                                   \
protected:                                                                                         \
  using Moose::Kokkos::TimeNodalKernel<T>::_u_dot;                                                 \
  using Moose::Kokkos::TimeNodalKernel<T>::_du_dot_du
