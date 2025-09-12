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
    _work_dofs(_count)
{
  addMooseVariableDependency(mooseVariable());
}

void
ArrayNodalKernel::prepareDofs()
{
  const dof_id_type root_dof_idx = _var.nodalDofIndex();
  std::iota(_work_dofs.begin(), _work_dofs.end(), root_dof_idx);
}

void
ArrayNodalKernel::computeResidual()
{
  if (!_var.isNodalDefined())
    return;
  prepareDofs();
  _qp = 0;
  computeQpResidual(_work_vector);
  addResiduals(_assembly, _work_vector, _work_dofs, _var.arrayScalingFactor());
}

void
ArrayNodalKernel::computeJacobian()
{
  if (!_var.isNodalDefined())
    return;
  prepareDofs();
  _qp = 0;
  const auto jacobian = computeQpJacobian();
  for (const auto i : make_range(_count))
    addJacobianElement(
        _assembly, jacobian(i), _work_dofs[i], _work_dofs[i], _var.arrayScalingFactor()[i]);
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
    prepareDofs();

    const auto & jvar = getVariable(jvar_num);
    const auto jacobian = computeQpOffDiagJacobian(jvar);
    const auto root_jvar_idx = _current_node->dof_number(_sys.number(), jvar_num, 0);

    for (const auto i : make_range(_var.count()))
      for (const auto j : make_range(jvar.count()))
        addJacobianElement(_assembly,
                           jacobian(i, j),
                           _work_dofs[i],
                           root_jvar_idx + j,
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
