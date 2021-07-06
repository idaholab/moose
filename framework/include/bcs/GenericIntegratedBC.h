//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"
#include "ADIntegratedBC.h"

template <bool is_ad>
class GenericIntegratedBC : public IntegratedBC
{
public:
  static InputParameters validParams() { return IntegratedBC::validParams(); };
  GenericIntegratedBC(const InputParameters & parameters) : IntegratedBC(parameters) {}
};

template <>
class GenericIntegratedBC<true> : public ADIntegratedBC
{
public:
  static InputParameters validParams() { return ADIntegratedBC::validParams(); };
  GenericIntegratedBC(const InputParameters & parameters) : ADIntegratedBC(parameters) {}
};

#define usingGenericIntegratedBCMembers                                                            \
  usingFunctionInterfaceMembers;                                                                   \
  usingPostprocessorInterfaceMembers;                                                              \
  usingMooseObjectMembers;                                                                         \
  usingTransientInterfaceMembers;                                                                  \
  usingTaggingInterfaceMembers;                                                                    \
  using GenericIntegratedBC<is_ad>::_qp;                                                           \
  using GenericIntegratedBC<is_ad>::_i;                                                            \
  using GenericIntegratedBC<is_ad>::_j;                                                            \
  using GenericIntegratedBC<is_ad>::_u;                                                            \
  using GenericIntegratedBC<is_ad>::_phi;                                                          \
  using GenericIntegratedBC<is_ad>::_test;                                                         \
  using GenericIntegratedBC<is_ad>::_q_point;                                                      \
  using GenericIntegratedBC<is_ad>::_var;                                                          \
  using GenericIntegratedBC<is_ad>::_name;                                                         \
  using GenericIntegratedBC<is_ad>::getVar;                                                        \
  using Coupleable::coupled;                                                                       \
  using Coupleable::coupledComponents
