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
    _work_vector(_count)
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
  addResiduals(_assembly, _work_vector, _var.dofIndices(), _var.arrayScalingFactor());
}

void
ArrayNodalKernel::computeJacobian()
{
  if (!_var.isNodalDefined())
    return;
  _qp = 0;
  const auto jacobian = computeQpJacobian();
  const auto & dof_indices = _var.dofIndices();
  mooseAssert(dof_indices.size() == _count, "The number of dofs should be equal to count");
  for (const auto i : make_range(_count))
    addJacobianElement(
        _assembly, jacobian(i), dof_indices[i], dof_indices[i], _var.arrayScalingFactor()[i]);
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
    const auto jacobian = computeQpOffDiagJacobian(jvar);
    const auto & ivar_indices = _var.dofIndices();
    const auto & jvar_indices = jvar.dofIndices();

    mooseAssert(ivar_indices.size() == _count, "The number of dofs should be equal to count");
    mooseAssert(jvar_indices.size() == jvar.count(), "The number of dofs should be equal to count");

    for (const auto i : make_range(_var.count()))
      for (const auto j : make_range(jvar.count()))
        addJacobianElement(_assembly,
                           jacobian(i, j),
                           ivar_indices[i],
                           jvar_indices[j],
                           _var.arrayScalingFactor()[i]);
  }
}

RealEigenVector
ArrayNodalKernel::computeQpJacobian()
{
  return RealEigenVector::Zero(_count);
}

RealEigenMatrix
ArrayNodalKernel::computeQpOffDiagJacobian(const MooseVariableFieldBase & jvar)
{
  return RealEigenMatrix::Zero(_count, jvar.count());
}
