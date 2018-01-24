//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimeNodalKernel.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariable.h"
#include "SystemBase.h"

template <>
InputParameters
validParams<TimeNodalKernel>()
{
  InputParameters params = validParams<NodalKernel>();
  return params;
}

TimeNodalKernel::TimeNodalKernel(const InputParameters & parameters) : NodalKernel(parameters) {}

void
TimeNodalKernel::computeResidual()
{
  if (_var.isNodalDefined())
  {
    dof_id_type & dof_idx = _var.nodalDofIndex();
    _qp = 0;
    Real res = computeQpResidual();
    _assembly.cacheResidualContribution(dof_idx, res, Moose::KT_TIME);

    if (_has_save_in)
    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      for (const auto & var : _save_in)
        var->sys().solution().add(var->nodalDofIndex(), res);
    }
  }
}
