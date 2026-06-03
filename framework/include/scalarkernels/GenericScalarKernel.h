//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ScalarKernel.h"
#include "ADScalarKernel.h"

template <bool is_ad>
class GenericScalarKernel : public ScalarKernel
{
public:
  static InputParameters validParams() { return ScalarKernel::validParams(); }
  GenericScalarKernel(const InputParameters & parameters) : ScalarKernel(parameters) {}
};

template <>
class GenericScalarKernel<true> : public ADScalarKernel
{
public:
  static InputParameters validParams() { return ADScalarKernel::validParams(); }
  GenericScalarKernel(const InputParameters & parameters) : ADScalarKernel(parameters) {}
};

#define usingGenericScalarKernelMembers                                                            \
  usingMooseObjectMembers;                                                                         \
  usingTransientInterfaceMembers;                                                                  \
  using GenericScalarKernel<is_ad>::_i;                                                            \
  using GenericScalarKernel<is_ad>::_j;                                                            \
  using GenericScalarKernel<is_ad>::_u;                                                            \
  using GenericScalarKernel<is_ad>::_var
