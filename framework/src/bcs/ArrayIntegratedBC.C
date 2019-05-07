//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayIntegratedBC.h"

#include "Assembly.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"

#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<ArrayIntegratedBC>()
{
  InputParameters params = validParams<IntegratedBCBase>();
  return params;
}

ArrayIntegratedBC::ArrayIntegratedBC(const InputParameters & parameters)
  : IntegratedBCBase(parameters),
    MooseVariableInterface<RealArrayValue>(this,
                                           false,
                                           "variable",
                                           Moose::VarKindType::VAR_NONLINEAR,
                                           Moose::VarFieldType::VAR_FIELD_ARRAY),
    _var(*mooseVariable()),
    _normals(_var.normals()),
    _phi(_assembly.phiFace(_var)),
    _test(_var.phiFace()),
    _u(_is_implicit ? _var.sln() : _var.slnOld()),
    _count(_var.count())
{
  addMooseVariableDependency(mooseVariable());
}

void
ArrayIntegratedBC::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < _test.size(); _i++)
    {
      RealArrayValue residual = _JxW[_qp] * _coord[_qp] * computeQpResidual();
      mooseAssert(residual.size() == _count,
                  "Size of local residual is not equal to the number of array variable compoments");
      saveLocalArrayResidual(_local_re, _i, _test.size(), residual);
    }

  accumulateTaggedLocalResidual();

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (const auto & var : _save_in)
    {
      auto * avar = dynamic_cast<ArrayMooseVariable *>(var);
      if (avar)
        avar->saveDoFValues(_local_re);
      else
        mooseError("Save-in variable for an array kernel must be an array variable");
    }
  }
}

void
ArrayIntegratedBC::computeJacobian()
{
  prepareMatrixTag(_assembly, _var.number(), _var.number());

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < _phi.size(); _j++)
      {
        RealArrayValue v = _JxW[_qp] * _coord[_qp] * computeQpJacobian();
        saveDiagLocalArrayJacobian(_local_ke, _i, _test.size(), _j, _phi.size(), v);
      }

  accumulateTaggedLocalMatrix();

  if (_has_diag_save_in)
  {
    unsigned int rows = _local_ke.m();
    DenseVector<Real> diag(rows);
    for (unsigned int i = 0; i < rows; i++)
      diag(i) = _local_ke(i, i);

    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (const auto & var : _diag_save_in)
    {
      auto * avar = dynamic_cast<ArrayMooseVariable *>(var);
      if (avar)
        avar->saveDoFValues(diag);
      else
        mooseError("Save-in variable for an array kernel must be an array variable");
    }
  }
}

void
ArrayIntegratedBC::computeJacobianBlock(MooseVariableFEBase & jvar)
{
  size_t jvar_num = jvar.number();
  prepareMatrixTag(_assembly, _var.number(), jvar_num);

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < jvar.phiFaceSize(); _j++)
      {
        RealArray v = _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(jvar);
        saveFullLocalArrayJacobian(_local_ke, _i, _test.size(), _j, jvar.phiSize(), v);
      }

  accumulateTaggedLocalMatrix();

  if (_has_diag_save_in && jvar.number() == _var.number())
  {
    unsigned int rows = _local_ke.m();
    DenseVector<Real> diag(rows);
    for (unsigned int i = 0; i < rows; i++)
      diag(i) = _local_ke(i, i);

    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (const auto & var : _diag_save_in)
    {
      auto * avar = dynamic_cast<ArrayMooseVariable *>(var);
      if (avar)
        avar->saveDoFValues(diag);
      else
        mooseError("Save-in variable for an array kernel must be an array variable");
    }
  }
}

void
ArrayIntegratedBC::computeJacobianBlockScalar(unsigned int jvar)
{
  prepareMatrixTag(_assembly, _var.number(), jvar);

  MooseVariableScalar & jv = _sys.getScalarVariable(_tid, jvar);
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < _test.size(); _i++)
    {
      RealArray v = _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobianScalar(jv);
      saveFullLocalArrayJacobian(_local_ke, _i, _test.size(), 0, 1, v);
    }

  accumulateTaggedLocalMatrix();
}
