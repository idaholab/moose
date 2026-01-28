//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayNodalKernel.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "MooseVariableFE.h"
#include "Assembly.h"

InputParameters
ArrayNodalKernel::validParams()
{
  return NodalKernelBase::validParams();
}

ArrayNodalKernel::ArrayNodalKernel(const InputParameters & parameters)
  : NodalKernelBase(parameters),
    MooseVariableInterface<RealEigenVector>(this,
                                            true,
                                            "variable",
                                            Moose::VarKindType::VAR_SOLVER,
                                            Moose::VarFieldType::VAR_FIELD_ARRAY),
    _var(*mooseVariable()),
    _u(_var.dofValues()),
    _count(_var.count()),
    _work_vector(_count),
    _scaling(_var.arrayScalingFactor())
{
  addMooseVariableDependency(mooseVariable());
}

void
ArrayNodalKernel::computeResidual()
{
  if (!_var.isNodalDefined())
    return;
  _qp = 0;
  computeQpResidual(_work_vector);
  addResiduals(_assembly, _work_vector, _var.dofIndices(), _scaling);
}

void
ArrayNodalKernel::computeJacobian()
{
  if (!_var.isNodalDefined())
    return;
  _qp = 0;

  _ivar_indices = &_var.dofIndices();
  _jvar_indices = &_var.dofIndices();
  mooseAssert(_ivar_indices->size() == _count, "The number of dofs should be equal to count");

  computeQpJacobian();
}

void
ArrayNodalKernel::computeOffDiagJacobian(const unsigned int jvar_num)
{
  if (!_var.isNodalDefined())
    return;

  if (jvar_num == _var.number())
    computeJacobian();
  else
  {
    const auto & jvar = getVariable(jvar_num);
    _ivar_indices = &_var.dofIndices();
    _jvar_indices = &jvar.dofIndices();
    mooseAssert(_ivar_indices->size() == _count, "The number of dofs should be equal to count");
    mooseAssert(_jvar_indices->size() == jvar.count(),
                "The number of dofs should be equal to count");

    computeQpOffDiagJacobian(jvar_num);
  }
}

void
ArrayNodalKernel::setJacobian(unsigned int i, unsigned int j, Real value)
{
  addJacobianElement(_assembly, value, (*_ivar_indices)[i], (*_jvar_indices)[j], _scaling[i]);
}
