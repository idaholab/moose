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

template <typename NodalKernel>
class GPUTimeNodalKernel : public GPUNodalKernel<NodalKernel>
{
  usingGPUNodalKernelMembers(NodalKernel);

public:
  static InputParameters validParams()
  {
    InputParameters params = GPUNodalKernel<NodalKernel>::validParams();

    params.set<MultiMooseEnum>("vector_tags") = "time";
    params.set<MultiMooseEnum>("matrix_tags") = "system time";

    return params;
  }

  GPUTimeNodalKernel(const InputParameters & parameters)
    : GPUNodalKernel<NodalKernel>(parameters),
      _u_dot(systems(), _var, Moose::SOLUTION_DOT_TAG),
      _du_dot_du(_var.sys().duDotDu(_var.number()))
  {
  }

protected:
  /// Time derivative of u
  GPUVariableNodalValue _u_dot;
  /// Derivative of u_dot with respect to u
  GPUScalar<const Real> _du_dot_du;
};

#define usingGPUTimeNodalKernelMembers(T)                                                          \
  usingGPUNodalKernelMembers(T);                                                                   \
                                                                                                   \
protected:                                                                                         \
  using GPUTimeNodalKernel<T>::_u_dot;                                                             \
  using GPUTimeNodalKernel<T>::_du_dot_du;
