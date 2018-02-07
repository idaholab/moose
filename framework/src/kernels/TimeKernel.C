//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimeKernel.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariableFEImpl.h"
#include "SystemBase.h"

#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<TimeKernel>()
{
  InputParameters params = validParams<Kernel>();

  params.set<MultiMooseEnum>("vector_tags") = "time";
  params.set<MultiMooseEnum>("matrix_tags") = "nontime";

  return params;
}

TimeKernel::TimeKernel(const InputParameters & parameters) : Kernel(parameters) {}

void
TimeKernel::computeResidual()
{
  _re_blocks.resize(_vector_tags.size());
  mooseAssert(_vector_tags.size() >= 1, "we need at least one active tag");
  auto vector_tag = _vector_tags.begin();
  for (auto i = beginIndex(_vector_tags); i < _vector_tags.size(); i++, ++vector_tag)
    _re_blocks[i] = &_assembly.residualBlock(_var.number(), *vector_tag);

  _local_re.resize(_re_blocks[0]->size());
  _local_re.zero();

  precalculateResidual();
  for (_i = 0; _i < _test.size(); _i++)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      _local_re(_i) += _JxW[_qp] * _coord[_qp] * computeQpResidual();

  for (auto & re : _re_blocks)
    *re += _local_re;

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i = 0; i < _save_in.size(); i++)
      _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());
  }
}
