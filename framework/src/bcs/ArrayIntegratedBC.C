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

InputParameters
ArrayIntegratedBC::validParams()
{
  InputParameters params = IntegratedBCBase::validParams();
  return params;
}

ArrayIntegratedBC::ArrayIntegratedBC(const InputParameters & parameters)
  : IntegratedBCBase(parameters),
    MooseVariableInterface<RealEigenVector>(this,
                                            false,
                                            "variable",
                                            Moose::VarKindType::VAR_NONLINEAR,
                                            Moose::VarFieldType::VAR_FIELD_ARRAY),
    _var(*mooseVariable()),
    _normals(_assembly.normals()),
    _phi(_assembly.phiFace(_var)),
    _test(_var.phiFace()),
    _u(_is_implicit ? _var.sln() : _var.slnOld()),
    _count(_var.count()),
    _work_vector(_count)
{
  addMooseVariableDependency(mooseVariable());

  _save_in.resize(_save_in_strings.size());
  _diag_save_in.resize(_diag_save_in_strings.size());

  for (unsigned int i = 0; i < _save_in_strings.size(); i++)
  {
    ArrayMooseVariable * var = &_subproblem.getArrayVariable(_tid, _save_in_strings[i]);

    if (var->feType() != _var.feType())
      paramError(
          "save_in",
          "saved-in auxiliary variable is incompatible with the object's nonlinear variable: ",
          moose::internal::incompatVarMsg(*var, _var));
    _save_in[i] = var;
    var->sys().addVariableToZeroOnResidual(_save_in_strings[i]);
    addMooseVariableDependency(var);
  }

  _has_save_in = _save_in.size() > 0;

  for (unsigned int i = 0; i < _diag_save_in_strings.size(); i++)
  {
    ArrayMooseVariable * var = &_subproblem.getArrayVariable(_tid, _diag_save_in_strings[i]);

    if (var->feType() != _var.feType())
      paramError(
          "diag_save_in",
          "saved-in auxiliary variable is incompatible with the object's nonlinear variable: ",
          moose::internal::incompatVarMsg(*var, _var));

    _diag_save_in[i] = var;
    var->sys().addVariableToZeroOnJacobian(_diag_save_in_strings[i]);
    addMooseVariableDependency(var);
  }

  _has_diag_save_in = _diag_save_in.size() > 0;
}

void
ArrayIntegratedBC::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    initQpResidual();
    for (_i = 0; _i < _test.size(); _i++)
    {
      _work_vector.setZero();
      computeQpResidual(_work_vector);
      mooseAssert(_work_vector.size() == _count,
                  "Size of local residual is not equal to the number of array variable compoments");
      _work_vector *= _JxW[_qp] * _coord[_qp];
      _assembly.saveLocalArrayResidual(_local_re, _i, _test.size(), _work_vector);
    }
  }

  accumulateTaggedLocalResidual();

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (const auto & var : _save_in)
    {
      auto * avar = dynamic_cast<ArrayMooseVariable *>(var);
      if (avar)
        avar->addSolution(_local_re);
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
  {
    initQpJacobian();
    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < _phi.size(); _j++)
      {
        RealEigenVector v = _JxW[_qp] * _coord[_qp] * computeQpJacobian();
        _assembly.saveDiagLocalArrayJacobian(
            _local_ke, _i, _test.size(), _j, _phi.size(), _var.number(), v);
      }
  }

  accumulateTaggedLocalMatrix();

  if (_has_diag_save_in)
  {
    DenseVector<Number> diag = _assembly.getJacobianDiagonal(_local_ke);
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (const auto & var : _diag_save_in)
    {
      auto * avar = dynamic_cast<ArrayMooseVariable *>(var);
      if (avar)
        avar->addSolution(diag);
      else
        mooseError("Save-in variable for an array kernel must be an array variable");
    }
  }
}

RealEigenVector
ArrayIntegratedBC::computeQpJacobian()
{
  return RealEigenVector::Zero(_var.count());
}

void
ArrayIntegratedBC::computeOffDiagJacobian(const unsigned int jvar_num)
{
  const auto & jvar = getVariable(jvar_num);

  bool same_var = jvar_num == _var.number();

  prepareMatrixTag(_assembly, _var.number(), jvar_num);

  // This (undisplaced) jvar could potentially yield the wrong phi size if this object is acting on
  // the displaced mesh
  auto phi_size = jvar.dofIndices().size();

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    initQpOffDiagJacobian(jvar);
    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < phi_size; _j++)
      {
        RealEigenMatrix v = _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(jvar);
        _assembly.saveFullLocalArrayJacobian(
            _local_ke, _i, _test.size(), _j, jvar.phiSize(), _var.number(), jvar_num, v);
      }
  }

  accumulateTaggedLocalMatrix();

  if (_has_diag_save_in && same_var)
  {
    DenseVector<Number> diag = _assembly.getJacobianDiagonal(_local_ke);
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (const auto & var : _diag_save_in)
    {
      auto * avar = dynamic_cast<ArrayMooseVariable *>(var);
      if (avar)
        avar->addSolution(diag);
      else
        mooseError("Save-in variable for an array kernel must be an array variable");
    }
  }
}

RealEigenMatrix
ArrayIntegratedBC::computeQpOffDiagJacobian(const MooseVariableFEBase & jvar)
{
  if (jvar.number() == _var.number())
    return computeQpJacobian().asDiagonal();
  else
    return RealEigenMatrix::Zero(_var.count(), jvar.count());
}

void
ArrayIntegratedBC::computeOffDiagJacobianScalar(unsigned int jvar)
{
  prepareMatrixTag(_assembly, _var.number(), jvar);

  const MooseVariableScalar & jv = _sys.getScalarVariable(_tid, jvar);
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < _test.size(); _i++)
    {
      RealEigenMatrix v = _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobianScalar(jv);
      _assembly.saveFullLocalArrayJacobian(
          _local_ke, _i, _test.size(), 0, 1, _var.number(), jvar, v);
    }

  accumulateTaggedLocalMatrix();
}

RealEigenMatrix
ArrayIntegratedBC::computeQpOffDiagJacobianScalar(const MooseVariableScalar & jvar)
{
  return RealEigenMatrix::Zero(_var.count(), (unsigned int)jvar.order() + 1);
}
