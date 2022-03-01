//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayDGKernel.h"
#include "Assembly.h"
#include "MooseVariable.h"
#include "Problem.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "MaterialData.h"
#include "ParallelUniqueId.h"

#include "libmesh/dof_map.h"
#include "libmesh/dense_vector.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/dense_subvector.h"
#include "libmesh/libmesh_common.h"
#include "libmesh/quadrature.h"

InputParameters
ArrayDGKernel::validParams()
{
  InputParameters params = DGKernelBase::validParams();
  return params;
}

ArrayDGKernel::ArrayDGKernel(const InputParameters & parameters)
  : DGKernelBase(parameters),
    NeighborMooseVariableInterface(
        this, false, Moose::VarKindType::VAR_NONLINEAR, Moose::VarFieldType::VAR_FIELD_ARRAY),
    _var(*mooseVariable()),
    _u(_is_implicit ? _var.sln() : _var.slnOld()),
    _grad_u(_is_implicit ? _var.gradSln() : _var.gradSlnOld()),

    _phi(_var.phiFace()),
    _grad_phi(_var.gradPhiFace()),
    _array_grad_phi(_var.arrayGradPhiFace()),

    _test(_var.phiFace()),
    _grad_test(_var.gradPhiFace()),
    _array_grad_test(_var.arrayGradPhiFace()),

    _phi_neighbor(_var.phiFaceNeighbor()),
    _grad_phi_neighbor(_var.gradPhiFaceNeighbor()),
    _array_grad_phi_neighbor(_var.arrayGradPhiFaceNeighbor()),

    _test_neighbor(_var.phiFaceNeighbor()),
    _grad_test_neighbor(_var.gradPhiFaceNeighbor()),
    _array_grad_test_neighbor(_var.arrayGradPhiFaceNeighbor()),

    _u_neighbor(_is_implicit ? _var.slnNeighbor() : _var.slnOldNeighbor()),
    _grad_u_neighbor(_is_implicit ? _var.gradSlnNeighbor() : _var.gradSlnOldNeighbor()),

    _array_normals(_assembly.mappedNormals()),
    _count(_var.count()),

    _work_vector(_count)
{
  addMooseVariableDependency(mooseVariable());

  _save_in.resize(_save_in_strings.size());
  _diag_save_in.resize(_diag_save_in_strings.size());

  for (unsigned int i = 0; i < _save_in_strings.size(); i++)
  {
    MooseVariableFEBase * var = &_subproblem.getVariable(_tid,
                                                         _save_in_strings[i],
                                                         Moose::VarKindType::VAR_AUXILIARY,
                                                         Moose::VarFieldType::VAR_FIELD_ARRAY);

    if (_sys.hasVariable(_save_in_strings[i]))
      mooseError("Trying to use solution variable " + _save_in_strings[i] +
                 " as a save_in variable in " + name());

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
    MooseVariableFEBase * var = &_subproblem.getVariable(_tid,
                                                         _diag_save_in_strings[i],
                                                         Moose::VarKindType::VAR_AUXILIARY,
                                                         Moose::VarFieldType::VAR_FIELD_ARRAY);

    if (_sys.hasVariable(_diag_save_in_strings[i]))
      mooseError("Trying to use solution variable " + _diag_save_in_strings[i] +
                 " as a diag_save_in variable in " + name());

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
ArrayDGKernel::computeElemNeighResidual(Moose::DGResidualType type)
{
  bool is_elem;
  if (type == Moose::Element)
    is_elem = true;
  else
    is_elem = false;

  const ArrayVariableTestValue & test_space = is_elem ? _test : _test_neighbor;

  if (is_elem)
    prepareVectorTag(_assembly, _var.number());
  else
    prepareVectorTagNeighbor(_assembly, _var.number());

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    initQpResidual(type);
    for (_i = 0; _i < test_space.size(); _i++)
    {
      _work_vector.setZero();
      computeQpResidual(type, _work_vector);
      mooseAssert(_work_vector.size() == _count,
                  "Size of local residual is not equal to the number of array variable compoments");
      _work_vector *= _JxW[_qp] * _coord[_qp];
      _assembly.saveLocalArrayResidual(_local_re, _i, test_space.size(), _work_vector);
    }
  }

  accumulateTaggedLocalResidual();

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(_resid_vars_mutex);
    for (const auto & var : _save_in)
    {
      auto * avar = dynamic_cast<ArrayMooseVariable *>(var);
      if (!avar)
        mooseError("Save-in variable for an array kernel must be an array variable");

      if (is_elem)
        avar->addSolution(_local_re);
      else
        avar->addSolutionNeighbor(_local_re);
    }
  }
}

void
ArrayDGKernel::computeElemNeighJacobian(Moose::DGJacobianType type)
{
  const ArrayVariableTestValue & test_space =
      (type == Moose::ElementElement || type == Moose::ElementNeighbor) ? _test : _test_neighbor;
  const ArrayVariableTestValue & loc_phi =
      (type == Moose::ElementElement || type == Moose::NeighborElement) ? _phi : _phi_neighbor;

  if (type == Moose::ElementElement)
    prepareMatrixTag(_assembly, _var.number(), _var.number());
  else
    prepareMatrixTagNeighbor(_assembly, _var.number(), _var.number(), type);

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    initQpJacobian(type);
    for (_i = 0; _i < test_space.size(); _i++)
      for (_j = 0; _j < loc_phi.size(); _j++)
      {
        RealEigenVector v = _JxW[_qp] * _coord[_qp] * computeQpJacobian(type);
        _assembly.saveDiagLocalArrayJacobian(
            _local_ke, _i, test_space.size(), _j, loc_phi.size(), _var.number(), v);
      }
  }

  accumulateTaggedLocalMatrix();

  if (_has_diag_save_in && (type == Moose::ElementElement || type == Moose::NeighborNeighbor))
  {
    DenseVector<Number> diag = _assembly.getJacobianDiagonal(_local_ke);
    Threads::spin_mutex::scoped_lock lock(_jacoby_vars_mutex);
    for (const auto & var : _diag_save_in)
    {
      auto * avar = dynamic_cast<ArrayMooseVariable *>(var);
      if (!avar)
        mooseError("Save-in variable for an array kernel must be an array variable");

      if (type == Moose::ElementElement)
        avar->addSolution(diag);
      else
        avar->addSolutionNeighbor(diag);
    }
  }
}

RealEigenVector ArrayDGKernel::computeQpJacobian(Moose::DGJacobianType)
{
  return RealEigenVector::Zero(_count);
}

void
ArrayDGKernel::computeOffDiagJacobian(const unsigned int jvar_num)
{
  if (!excludeBoundary())
  {
    const auto & jvar = getVariable(jvar_num);

    precalculateOffDiagJacobian(jvar_num);

    // Compute element-element Jacobian
    computeOffDiagElemNeighJacobian(Moose::ElementElement, jvar);

    // Compute element-neighbor Jacobian
    computeOffDiagElemNeighJacobian(Moose::ElementNeighbor, jvar);

    // Compute neighbor-element Jacobian
    computeOffDiagElemNeighJacobian(Moose::NeighborElement, jvar);

    // Compute neighbor-neighbor Jacobian
    computeOffDiagElemNeighJacobian(Moose::NeighborNeighbor, jvar);
  }
}

void
ArrayDGKernel::computeOffDiagElemNeighJacobian(Moose::DGJacobianType type,
                                               const MooseVariableFEBase & jvar)
{
  const ArrayVariableTestValue & test_space =
      (type == Moose::ElementElement || type == Moose::ElementNeighbor) ? _test : _test_neighbor;

  if (type == Moose::ElementElement)
    prepareMatrixTag(_assembly, _var.number(), jvar.number());
  else
    prepareMatrixTagNeighbor(_assembly, _var.number(), jvar.number(), type);

  if (_local_ke.n() == 0 || _local_ke.m() == 0)
    return;

  if (jvar.fieldType() == Moose::VarFieldType::VAR_FIELD_STANDARD)
  {
    const auto & jv0 = static_cast<const MooseVariable &>(jvar);
    const VariableTestValue & loc_phi =
        (type == Moose::ElementElement || type == Moose::NeighborElement) ? jv0.phiFace()
                                                                          : jv0.phiFaceNeighbor();

    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      initQpOffDiagJacobian(type, jvar);
      for (_i = 0; _i < test_space.size(); _i++)
        for (_j = 0; _j < loc_phi.size(); _j++)
        {
          RealEigenMatrix v = _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(type, jvar);
          _assembly.saveFullLocalArrayJacobian(_local_ke,
                                               _i,
                                               test_space.size(),
                                               _j,
                                               loc_phi.size(),
                                               _var.number(),
                                               jvar.number(),
                                               v);
        }
    }
  }
  else if (jvar.fieldType() == Moose::VarFieldType::VAR_FIELD_ARRAY)
  {
    const auto & jv1 = static_cast<const ArrayMooseVariable &>(jvar);
    const ArrayVariableTestValue & loc_phi =
        (type == Moose::ElementElement || type == Moose::NeighborElement) ? jv1.phiFace()
                                                                          : jv1.phiFaceNeighbor();

    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      initQpOffDiagJacobian(type, jvar);
      for (_i = 0; _i < test_space.size(); _i++)
        for (_j = 0; _j < loc_phi.size(); _j++)
        {
          RealEigenMatrix v = _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(type, jvar);
          _assembly.saveFullLocalArrayJacobian(_local_ke,
                                               _i,
                                               test_space.size(),
                                               _j,
                                               loc_phi.size(),
                                               _var.number(),
                                               jvar.number(),
                                               v);
        }
    }
  }
  else
    mooseError("Vector variable cannot be coupled into array DG kernel currently");

  accumulateTaggedLocalMatrix();

  if (_has_diag_save_in && (type == Moose::ElementElement || type == Moose::NeighborNeighbor) &&
      _var.number() == jvar.number())
  {
    DenseVector<Number> diag = _assembly.getJacobianDiagonal(_local_ke);
    Threads::spin_mutex::scoped_lock lock(_jacoby_vars_mutex);
    for (const auto & var : _diag_save_in)
    {
      auto * avar = dynamic_cast<ArrayMooseVariable *>(var);
      if (!avar)
        mooseError("Save-in variable for an array kernel must be an array variable");

      if (type == Moose::ElementElement)
        avar->addSolution(diag);
      else
        avar->addSolutionNeighbor(diag);
    }
  }
}

RealEigenMatrix
ArrayDGKernel::computeQpOffDiagJacobian(Moose::DGJacobianType type,
                                        const MooseVariableFEBase & jvar)
{
  if (jvar.number() == _var.number())
    return computeQpJacobian(type).asDiagonal();
  else
    return RealEigenMatrix::Zero(_var.count(), jvar.count());
}
