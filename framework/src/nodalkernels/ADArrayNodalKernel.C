//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADArrayNodalKernel.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "MooseVariableFE.h"
#include "Assembly.h"

InputParameters
ADArrayNodalKernel::validParams()
{
  return NodalKernelBase::validParams();
}

ADArrayNodalKernel::ADArrayNodalKernel(const InputParameters & parameters)
  : NodalKernelBase(parameters),
    MooseVariableInterface<RealEigenVector>(this,
                                            true,
                                            "variable",
                                            Moose::VarKindType::VAR_SOLVER,
                                            Moose::VarFieldType::VAR_FIELD_ARRAY),
    _var(*mooseVariable()),
    _u(_var.adDofValues()),
    _count(_var.count()),
    _work_vector(_count)
{
  addMooseVariableDependency(mooseVariable());
}

void
ADArrayNodalKernel::computeResidual()
{
  if (!_var.isNodalDefined())
    return;
  _qp = 0;
  computeQpResidual(_work_vector);
  addResiduals(_assembly,
               MetaPhysicL::raw_value(_work_vector),
               _var.dofIndices(),
               _var.arrayScalingFactor());
}

void
ADArrayNodalKernel::computeJacobian()
{
  if (!_var.isNodalDefined())
    return;
  _qp = 0;
  computeQpResidual(_work_vector);
  addJacobian(_assembly, _work_vector, _var.dofIndices(), _var.arrayScalingFactor());
}

void
ADArrayNodalKernel::computeOffDiagJacobian(const unsigned int)
{
  if (_my_node != _current_node)
  {
    computeJacobian();
    _my_node = _current_node;
  }
}
