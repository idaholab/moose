//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayNodalBC.h"

#include "Assembly.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"
#include "NonlinearSystemBase.h"

InputParameters
ArrayNodalBC::validParams()
{
  InputParameters params = NodalBCBase::validParams();
  return params;
}

ArrayNodalBC::ArrayNodalBC(const InputParameters & parameters)
  : NodalBCBase(parameters),
    MooseVariableInterface<RealEigenVector>(this,
                                            true,
                                            "variable",
                                            Moose::VarKindType::VAR_NONLINEAR,
                                            Moose::VarFieldType::VAR_FIELD_ARRAY),
    _var(*mooseVariable()),
    _current_node(_var.node()),
    _u(_var.nodalValue()),
    _count(_var.count()),
    _work_vector(_count)
{
  addMooseVariableDependency(mooseVariable());
}

void
ArrayNodalBC::computeResidual()
{
  if (_var.isNodalDefined())
  {
    _work_vector.setZero();
    computeQpResidual(_work_vector);
    mooseAssert(_work_vector.size() == _count,
                "Size of local residual is not equal to the number of array variable components");

    for (auto tag_id : _vector_tags)
      if (_sys.hasVector(tag_id))
        _var.insertNodalValue(_sys.getVector(tag_id), _work_vector);
  }
}

void
ArrayNodalBC::computeJacobian()
{
  if (_var.isNodalDefined())
  {
    RealEigenVector cached_val = computeQpJacobian();

    dof_id_type cached_row = _var.nodalDofIndex();

    // Cache the user's computeQpJacobian() value for later use.
    for (auto tag : _matrix_tags)
      if (_sys.hasMatrix(tag))
        for (unsigned int i = 0; i < _var.count(); ++i)
          _fe_problem.assembly(0, _sys.number())
              .cacheJacobian(cached_row + i, cached_row + i, cached_val(i), tag);
  }
}

void
ArrayNodalBC::computeOffDiagJacobian(const unsigned int jvar_num)
{
  if (!_var.isNodalDefined())
    return;

  const auto & jvar = getVariable(jvar_num);

  RealEigenMatrix cached_val = computeQpOffDiagJacobian(const_cast<MooseVariableFieldBase &>(jvar));

  dof_id_type cached_row = _var.nodalDofIndex();
  // Note: this only works for Lagrange variables...
  dof_id_type cached_col = _current_node->dof_number(_sys.number(), jvar_num, 0);

  // Cache the user's computeQpJacobian() value for later use.
  for (auto tag : _matrix_tags)
    if (_sys.hasMatrix(tag))
      for (unsigned int i = 0; i < _var.count(); ++i)
        for (unsigned int j = 0; j < jvar.count(); ++j)
          _fe_problem.assembly(0, _sys.number())
              .cacheJacobian(cached_row + i, cached_col + j, cached_val(i, j), tag);
}

RealEigenVector
ArrayNodalBC::computeQpJacobian()
{
  return RealEigenVector::Ones(_var.count());
  ;
}

RealEigenMatrix
ArrayNodalBC::computeQpOffDiagJacobian(MooseVariableFEBase & jvar)
{
  if (jvar.number() == _var.number())
    return RealEigenMatrix::Identity(_var.count(), jvar.count());
  else
    return RealEigenMatrix::Zero(_var.count(), jvar.count());
}
