//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "NodeElemConstraint.h"
#include "ADNodeElemConstraint.h"

template <bool is_ad>
class GenericNodeElemConstraint : public NodeElemConstraint
{
public:
  static InputParameters validParams() { return NodeElemConstraint::validParams(); };
  GenericNodeElemConstraint(const InputParameters & parameters) : NodeElemConstraint(parameters) {}
};

template <>
class GenericNodeElemConstraint<true> : public ADNodeElemConstraint
{
public:
  static InputParameters validParams() { return ADNodeElemConstraint::validParams(); };
  GenericNodeElemConstraint(const InputParameters & parameters) : ADNodeElemConstraint(parameters)
  {
  }
};

#define usingGenericNodeElemConstraint                                                             \
  using GenericNodeElemConstraint<is_ad>::_mesh;                                                   \
  using GenericNodeElemConstraint<is_ad>::_secondary;                                              \
  using GenericNodeElemConstraint<is_ad>::_primary;                                                \
  using GenericNodeElemConstraint<is_ad>::_secondary_to_primary_map;                               \
  using GenericNodeElemConstraint<is_ad>::_current_node;                                           \
  using GenericNodeElemConstraint<is_ad>::_u_secondary;                                            \
  using GenericNodeElemConstraint<is_ad>::_u_primary;                                              \
  using GenericNodeElemConstraint<is_ad>::_j;                                                      \
  using GenericNodeElemConstraint<is_ad>::_i;                                                      \
  using GenericNodeElemConstraint<is_ad>::_jacobian;                                               \
  using GenericNodeElemConstraint<is_ad>::_phi_secondary;                                          \
  using GenericNodeElemConstraint<is_ad>::_phi_primary;                                            \
  using GenericNodeElemConstraint<is_ad>::_var;                                                    \
  using GenericNodeElemConstraint<is_ad>::_qp;                                                     \
  using GenericNodeElemConstraint<is_ad>::_subproblem;                                             \
  using GenericNodeElemConstraint<is_ad>::_test_primary;                                           \
  using GenericNodeElemConstraint<is_ad>::_test_secondary;                                         \
  using GenericNodeElemConstraint<is_ad>::_connected_dof_indices;                                  \
  using GenericNodeElemConstraint<is_ad>::_sys;                                                    \
  using GenericNodeElemConstraint<is_ad>::_overwrite_secondary_residual
