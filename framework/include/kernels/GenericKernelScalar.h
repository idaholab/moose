//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KernelScalarBase.h"
#include "ADKernelScalarBase.h"

template <bool is_ad>
class GenericKernelScalar : public KernelScalarBase
{
public:
  static InputParameters validParams() { return KernelScalarBase::validParams(); };
  GenericKernelScalar(const InputParameters & parameters) : KernelScalarBase(parameters) {}
};

template <>
class GenericKernelScalar<true> : public ADKernelScalarBase
{
public:
  static InputParameters validParams() { return ADKernelScalarBase::validParams(); };
  GenericKernelScalar(const InputParameters & parameters) : ADKernelScalarBase(parameters) {}
};

#define usingGenericKernelScalarMembers                                                            \
  usingFunctionInterfaceMembers;                                                                   \
  usingPostprocessorInterfaceMembers;                                                              \
  usingMooseObjectMembers;                                                                         \
  usingTransientInterfaceMembers;                                                                  \
  usingTaggingInterfaceMembers;                                                                    \
  usingBlockRestrictableMembers;                                                                   \
  using GenericKernelScalar<is_ad>::_qp;                                                           \
  using GenericKernelScalar<is_ad>::_i;                                                            \
  using GenericKernelScalar<is_ad>::_j;                                                            \
  using GenericKernelScalar<is_ad>::_u;                                                            \
  using GenericKernelScalar<is_ad>::_phi;                                                          \
  using GenericKernelScalar<is_ad>::_test;                                                         \
  using GenericKernelScalar<is_ad>::_q_point;                                                      \
  using GenericKernelScalar<is_ad>::_var;                                                          \
  using GenericKernelScalar<is_ad>::_name;                                                         \
  using GenericKernelScalar<is_ad>::getVar;                                                        \
  using GenericKernelScalar<is_ad>::_h;                                                            \
  using GenericKernelScalar<is_ad>::_l;                                                            \
  using GenericKernelScalar<is_ad>::_compute_scalar_residuals;                                     \
  using GenericKernelScalar<is_ad>::_compute_field_residuals;                                      \
  using GenericKernelScalar<is_ad>::_kappa_var;                                                    \
  using GenericKernelScalar<is_ad>::_k_order;                                                      \
  using GenericKernelScalar<is_ad>::_kappa;                                                        \
  using Coupleable::coupled;                                                                       \
  using Coupleable::coupledComponents
