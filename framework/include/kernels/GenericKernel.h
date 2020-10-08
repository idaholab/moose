//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"
#include "ADKernel.h"

template <bool is_ad>
class GenericKernel : public Kernel
{
public:
  static InputParameters validParams() { return Kernel::validParams(); };
  GenericKernel(const InputParameters & parameters) : Kernel(parameters) {}
};

template <>
class GenericKernel<true> : public ADKernel
{
public:
  static InputParameters validParams() { return ADKernel::validParams(); };
  GenericKernel(const InputParameters & parameters) : ADKernel(parameters) {}
};

#define usingGenericKernelMembers                                                                  \
  using GenericKernel<is_ad>::_qp;                                                                 \
  using GenericKernel<is_ad>::_i;                                                                  \
  using GenericKernel<is_ad>::_t;                                                                  \
  using GenericKernel<is_ad>::_test;                                                               \
  using GenericKernel<is_ad>::_q_point;                                                            \
  usingFunctionInterfaceMembers;                                                                   \
  usingPostprocessorInterfaceMembers
