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
  usingCoupleableMembers;                                                                          \
  using GenericKernelGrad<is_ad>::_qp;                                                             \
  using GenericKernelGrad<is_ad>::_grad_u
