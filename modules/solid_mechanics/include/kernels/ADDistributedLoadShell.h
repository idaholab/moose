//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericKernel.h"
#include "MooseTypes.h"
#include "MooseTypes.h"
#include "libmesh/quadrature_gauss.h"

class Function;

/**
 * ADDistributedLoadShell applies a distributed load on the shell element in a given direction
 * defined by component or normal to the shell plane
 */

template <bool is_ad>
class ADDistributedLoadShellTempl : public GenericKernel<is_ad>
{
public:
  static InputParameters validParams();

  ADDistributedLoadShellTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;

  GenericReal<is_ad> computeFactor() const;

  /// Number of displacement variables
  const unsigned int _ndisp;
  /// displacement component to apply the pressure to the shell
  unsigned int _component;
  /// distributed load value defined through function
  const Function * const _function;

  ///if this parameter is set true, the load will be applied normal to the shell plane
  const bool _project_load_to_normal;

  /// Variable numbers of coupled displacement variables
  std::vector<unsigned int> _disp_var;

  /// Vector storing pointers to the nodes of the shell element
  std::vector<const Node *> _nodes;

  /// An auxiliary in-plane vector used to calculate the normal vector to the shell
  RealVectorValue _v1;
  /// An auxiliary in-plane vector used to calculate the normal vector to the shell
  RealVectorValue _v2;
  /// Normal vector to the shell plane calcualted from cross product of _v1 and _v2
  RealVectorValue _normal;

  usingTransientInterfaceMembers;
  using GenericKernel<is_ad>::_i;
  using GenericKernel<is_ad>::_name;
  using GenericKernel<is_ad>::_current_elem;
  using GenericKernel<is_ad>::_q_point;
  using GenericKernel<is_ad>::_qp;
  using GenericKernel<is_ad>::_test;
  using GenericKernel<is_ad>::_var;
};

class ADDistributedLoadShell : public ADDistributedLoadShellTempl<false>
{
public:
  using ADDistributedLoadShellTempl<false>::ADDistributedLoadShellTempl;

protected:
};

typedef ADDistributedLoadShellTempl<true> ADADDistributedLoadShell;
