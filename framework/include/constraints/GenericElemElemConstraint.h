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
#include "ElemElemConstraint.h"
#include "ADElemElemConstraint.h"

template <bool is_ad>
class GenericElemElemConstraint : public ElemElemConstraint
{
public:
  static InputParameters validParams() { return ElemElemConstraint::validParams(); };
  GenericElemElemConstraint(const InputParameters & parameters) : ElemElemConstraint(parameters) {}

protected:
};

template <>
class GenericElemElemConstraint<true> : public ADElemElemConstraint
{
public:
  static InputParameters validParams() { return ADElemElemConstraint::validParams(); };
  GenericElemElemConstraint(const InputParameters & parameters) : ADElemElemConstraint(parameters)
  {
  }

protected:
};

#define usingGenericElemElemConstraint                                                             \
  using GenericElemElemConstraint<is_ad>::_mesh;                                                   \
  using GenericElemElemConstraint<is_ad>::_fe_problem;                                             \
  using GenericElemElemConstraint<is_ad>::_dim;                                                    \
  using GenericElemElemConstraint<is_ad>::_interface_id;                                           \
  using GenericElemElemConstraint<is_ad>::_var;                                                    \
  using GenericElemElemConstraint<is_ad>::_current_elem;                                           \
  using GenericElemElemConstraint<is_ad>::_neighbor_elem;                                          \
  using GenericElemElemConstraint<is_ad>::_constraint_q_point;                                     \
  using GenericElemElemConstraint<is_ad>::_constraint_weight;                                      \
  using GenericElemElemConstraint<is_ad>::_i;                                                      \
  using GenericElemElemConstraint<is_ad>::_j;                                                      \
  using GenericElemElemConstraint<is_ad>::_u;                                                      \
  using GenericElemElemConstraint<is_ad>::_grad_u;                                                 \
  using GenericElemElemConstraint<is_ad>::_phi;                                                    \
  using GenericElemElemConstraint<is_ad>::_grad_phi;                                               \
  using GenericElemElemConstraint<is_ad>::_test;                                                   \
  using GenericElemElemConstraint<is_ad>::_grad_test;                                              \
  using GenericElemElemConstraint<is_ad>::_phi_neighbor;                                           \
  using GenericElemElemConstraint<is_ad>::_grad_phi_neighbor;                                      \
  using GenericElemElemConstraint<is_ad>::_test_neighbor;                                          \
  using GenericElemElemConstraint<is_ad>::_grad_test_neighbor;                                     \
  using GenericElemElemConstraint<is_ad>::_u_neighbor;                                             \
  using GenericElemElemConstraint<is_ad>::_grad_u_neighbor;                                        \
  using GenericElemElemConstraint<is_ad>::_qp;                                                     \
  using GenericElemElemConstraint<is_ad>::_t;                                                      \
  using GenericElemElemConstraint<is_ad>::_assembly;                                               \
  using GenericElemElemConstraint<is_ad>::_local_re;                                               \
  using GenericElemElemConstraint<is_ad>::_local_ke;                                               \
  using GenericElemElemConstraint<is_ad>::_sys;                                                    \
  using GenericElemElemConstraint<is_ad>::_tid;                                                    \
  using GenericElemElemConstraint<is_ad>::_subproblem;                                             \
  using GenericElemElemConstraint<is_ad>::prepareVectorTag;                                        \
  using GenericElemElemConstraint<is_ad>::prepareVectorTagNeighbor;                                \
  using GenericElemElemConstraint<is_ad>::prepareMatrixTagNeighbor;                                \
  using GenericElemElemConstraint<is_ad>::accumulateTaggedLocalResidual;                           \
  using GenericElemElemConstraint<is_ad>::accumulateTaggedLocalMatrix;                             \
  using GenericElemElemConstraint<is_ad>::addMooseVariableDependency
