//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KernelCurl.h"
#include "ADKernelCurl.h"

template <bool is_ad>
class GenericKernelCurl : public KernelCurl
{
public:
  static InputParameters validParams() { return KernelCurl::validParams(); };
  GenericKernelCurl(const InputParameters & parameters) : KernelCurl(parameters) {}
};

template <>
class GenericKernelCurl<true> : public ADKernelCurl
{
public:
  static InputParameters validParams() { return ADKernelCurl::validParams(); };
  GenericKernelCurl(const InputParameters & parameters) : ADKernelCurl(parameters) {}
};

#define usingGenericKernelCurlMembers                                                              \
  usingFunctionInterfaceMembers;                                                                   \
  usingPostprocessorInterfaceMembers;                                                              \
  usingMooseObjectMembers;                                                                         \
  usingTransientInterfaceMembers;                                                                  \
  usingTaggingInterfaceMembers;                                                                    \
  usingBlockRestrictableMembers;                                                                   \
  using GenericKernelCurl<is_ad>::_qp;                                                             \
  using GenericKernelCurl<is_ad>::_i;                                                              \
  using GenericKernelCurl<is_ad>::_j;                                                              \
  using GenericKernelCurl<is_ad>::_u;                                                              \
  using GenericKernelCurl<is_ad>::_phi;                                                            \
  using GenericKernelCurl<is_ad>::_test;                                                           \
  using GenericKernelCurl<is_ad>::_q_point;                                                        \
  using GenericKernelCurl<is_ad>::_var;                                                            \
  using GenericKernelCurl<is_ad>::getVar;                                                          \
  using GenericKernelCurl<is_ad>::_curl_test;                                                      \
  using GenericKernelCurl<is_ad>::_curl_phi;                                                       \
  using GenericKernelCurl<is_ad>::_curl_u;                                                         \
  using Coupleable::coupled;                                                                       \
  using Coupleable::coupledComponents
