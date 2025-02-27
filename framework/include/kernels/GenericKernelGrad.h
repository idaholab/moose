//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KernelGrad.h"
#include "ADKernelGrad.h"

template <bool is_ad>
using GenericKernelGrad = std::conditional_t<is_ad, ADKernelGrad, KernelGrad>;

#define usingGenericKernelGradMembers                                                              \
  usingFunctionInterfaceMembers;                                                                   \
  usingPostprocessorInterfaceMembers;                                                              \
  usingMooseObjectMembers;                                                                         \
  usingTransientInterfaceMembers;                                                                  \
  usingTaggingInterfaceMembers;                                                                    \
  usingBlockRestrictableMembers;                                                                   \
  using GenericKernelGrad<is_ad>::_qp;                                                             \
  using GenericKernelGrad<is_ad>::_i;                                                              \
  using GenericKernelGrad<is_ad>::_j;                                                              \
  using GenericKernelGrad<is_ad>::_u;                                                              \
  using GenericKernelGrad<is_ad>::_phi;                                                            \
  using GenericKernelGrad<is_ad>::_test;                                                           \
  using GenericKernelGrad<is_ad>::_grad_test;                                                      \
  using GenericKernelGrad<is_ad>::_q_point;                                                        \
  using GenericKernelGrad<is_ad>::_var;                                                            \
  using GenericKernelGrad<is_ad>::_coupled_moose_vars;                                             \
  using GenericKernelGrad<is_ad>::_grad_u;                                                         \
  using GenericKernelGrad<is_ad>::_grad_phi;                                                       \
  using GenericKernelGrad<is_ad>::getVar;                                                          \
  using Coupleable::coupled;                                                                       \
  using Coupleable::isCoupled;                                                                     \
  using Coupleable::coupledComponents
