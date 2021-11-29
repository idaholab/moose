//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorIntegratedBC.h"
#include "Assembly.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"

#include "libmesh/quadrature.h"

InputParameters
VectorIntegratedBC::validParams()
{
  InputParameters params = IntegratedBCBase::validParams();
  return params;
}

VectorIntegratedBC::VectorIntegratedBC(const InputParameters & parameters)
  : IntegratedBCBase(parameters),
    MooseVariableInterface<RealVectorValue>(this,
                                            false,
                                            "variable",
                                            Moose::VarKindType::VAR_NONLINEAR,
                                            Moose::VarFieldType::VAR_FIELD_VECTOR),
    _var(*mooseVariable()),
    _normals(_assembly.normals()),
    _phi(_assembly.phiFace(_var)),
    _test(_var.phiFace()),
    _u(_is_implicit ? _var.sln() : _var.slnOld())
{
  addMooseVariableDependency(mooseVariable());
}

void
VectorIntegratedBC::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < _test.size(); _i++)
    {
      Real residual = _JxW[_qp] * _coord[_qp] * computeQpResidual();
      _local_re(_i) += residual;
    }

  accumulateTaggedLocalResidual();
}

void
VectorIntegratedBC::computeJacobian()
{
  prepareMatrixTag(_assembly, _var.number(), _var.number());

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < _phi.size(); _j++)
        _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpJacobian();

  accumulateTaggedLocalMatrix();
}

void
VectorIntegratedBC::computeOffDiagJacobian(const unsigned int jvar_num)
{
  const auto & jvar = getVariable(jvar_num);

  prepareMatrixTag(_assembly, _var.number(), jvar_num);

  // This (undisplaced) jvar could potentially yield the wrong phi size if this object is acting on
  // the displaced mesh
  auto phi_size = jvar.dofIndices().size();

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < phi_size; _j++)
      {
        if (_var.number() == jvar_num)
          _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpJacobian();
        else
          _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(jvar_num);
      }

  accumulateTaggedLocalMatrix();
}

void
VectorIntegratedBC::computeOffDiagJacobianScalar(unsigned int jvar)
{
  prepareMatrixTag(_assembly, _var.number(), jvar);

  MooseVariableScalar & jv = _sys.getScalarVariable(_tid, jvar);
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < jv.order(); _j++)
        _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobianScalar(jvar);

  accumulateTaggedLocalMatrix();
}
