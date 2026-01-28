//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ArrayNodalKernel.h"
#include "ADArrayNodalKernel.h"

/**
 * Template for array nodal kernels, using or not automatic differentiation
 */
template <bool is_ad>
class GenericArrayNodalKernel : public ArrayNodalKernel
{
public:
  static InputParameters validParams() { return ArrayNodalKernel::validParams(); }
  GenericArrayNodalKernel(const InputParameters & parameters) : ArrayNodalKernel(parameters) {}
};

template <>
class GenericArrayNodalKernel<true> : public ADArrayNodalKernel
{
public:
  static InputParameters validParams() { return ADArrayNodalKernel::validParams(); }
  GenericArrayNodalKernel(const InputParameters & parameters) : ADArrayNodalKernel(parameters) {}
};

#define usingGenericArrayNodalKernelMembers                                                        \
  using GenericArrayNodalKernel<is_ad>::_count;                                                    \
  using GenericArrayNodalKernel<is_ad>::_u;                                                        \
  using GenericArrayNodalKernel<is_ad>::_qp;                                                       \
  using GenericArrayNodalKernel<is_ad>::setJacobian;                                               \
  using GenericArrayNodalKernel<is_ad>::paramError
