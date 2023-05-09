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
#include "MooseVariableFE.h"
#include "SystemBase.h"

InputParameters
TimeNodalKernel::validParams()
{
  InputParameters params = NodalKernel::validParams();

  params.set<MultiMooseEnum>("vector_tags") = "time";
  params.set<MultiMooseEnum>("matrix_tags") = "system time";

  return params;
}

TimeNodalKernel::TimeNodalKernel(const InputParameters & parameters)
  : NodalKernel(parameters), _u_dot(_var.dofValuesDot()), _du_dot_du(_var.dofValuesDuDotDu())
{
}

void
TimeNodalKernel::computeResidual()
{
  if (_var.isNodalDefined())
  {
    const dof_id_type & dof_idx = _var.nodalDofIndex();
    _qp = 0;
    const Real res = computeQpResidual();
    addResiduals(_assembly,
                 std::array<Real, 1>{{res}},
                 std::array<dof_id_type, 1>{{dof_idx}},
                 _var.scalingFactor());

    if (_has_save_in)
      for (const auto & var : _save_in)
        var->sys().solution().add(var->nodalDofIndex(), res);
  }
}
