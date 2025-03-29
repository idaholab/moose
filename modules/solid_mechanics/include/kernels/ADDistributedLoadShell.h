//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
 * ADDistributedLoadShell applies a Distributed Load on the shell element in a given direction
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
  /// distributed load value defined through constant factor, function*factor, and postprocessor*factor
  const Real _factor;
  const Function * const _function;
  const PostprocessorValue * const _postprocessor;

  ///if this parameter is set true, the load will be normal to shell plane (default: false)
  const bool _project_load_to_normal;

  /// Variable numbers of coupled displacement variables
  std::vector<unsigned int> _disp_var;

  /// Vector storing pointers to the nodes of the shell element
  std::vector<const Node *> _nodes;

  RealVectorValue _v1;
  RealVectorValue _v2;
  RealVectorValue _normal;

  const bool _use_displaced_mesh;

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
