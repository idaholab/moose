//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DGKernel.h"
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
DGKernel::validParams()
{
  InputParameters params = DGKernelBase::validParams();
  return params;
}

DGKernel::DGKernel(const InputParameters & parameters)
  : DGKernelBase(parameters),
    NeighborMooseVariableInterface(
        this, false, Moose::VarKindType::VAR_NONLINEAR, Moose::VarFieldType::VAR_FIELD_STANDARD),
    _var(*mooseVariable()),
    _u(_is_implicit ? _var.sln() : _var.slnOld()),
    _grad_u(_is_implicit ? _var.gradSln() : _var.gradSlnOld()),

    _phi(_assembly.phiFace(_var)),
    _grad_phi(_assembly.gradPhiFace(_var)),

    _test(_var.phiFace()),
    _grad_test(_var.gradPhiFace()),

    _phi_neighbor(_assembly.phiFaceNeighbor(_var)),
    _grad_phi_neighbor(_assembly.gradPhiFaceNeighbor(_var)),

    _test_neighbor(_var.phiFaceNeighbor()),
    _grad_test_neighbor(_var.gradPhiFaceNeighbor()),

    _u_neighbor(_is_implicit ? _var.slnNeighbor() : _var.slnOldNeighbor()),
    _grad_u_neighbor(_is_implicit ? _var.gradSlnNeighbor() : _var.gradSlnOldNeighbor())
{
  addMooseVariableDependency(mooseVariable());

  _save_in.resize(_save_in_strings.size());
  _diag_save_in.resize(_diag_save_in_strings.size());

  for (unsigned int i = 0; i < _save_in_strings.size(); i++)
  {
    MooseVariableFEBase * var = &_subproblem.getVariable(_tid,
                                                         _save_in_strings[i],
                                                         Moose::VarKindType::VAR_AUXILIARY,
                                                         Moose::VarFieldType::VAR_FIELD_STANDARD);

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
                                                         Moose::VarFieldType::VAR_FIELD_STANDARD);

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
DGKernel::computeElemNeighResidual(Moose::DGResidualType type)
{
  bool is_elem;
  if (type == Moose::Element)
    is_elem = true;
  else
    is_elem = false;

  const VariableTestValue & test_space = is_elem ? _test : _test_neighbor;

  if (is_elem)
    prepareVectorTag(_assembly, _var.number());
  else
    prepareVectorTagNeighbor(_assembly, _var.number());

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    precalculateQpResidual(type);
    for (_i = 0; _i < test_space.size(); _i++)
      _local_re(_i) += _JxW[_qp] * _coord[_qp] * computeQpResidual(type);
  }

  accumulateTaggedLocalResidual();

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(_resid_vars_mutex);
    for (const auto & var : _save_in)
    {
      const std::vector<dof_id_type> & dof_indices =
          is_elem ? var->dofIndices() : var->dofIndicesNeighbor();
      var->sys().solution().add_vector(_local_re, dof_indices);
    }
  }
}

void
DGKernel::computeElemNeighJacobian(Moose::DGJacobianType type)
{
  const VariableTestValue & test_space =
      (type == Moose::ElementElement || type == Moose::ElementNeighbor) ? _test : _test_neighbor;
  const VariableTestValue & loc_phi =
      (type == Moose::ElementElement || type == Moose::NeighborElement) ? _phi : _phi_neighbor;

  if (type == Moose::ElementElement)
    prepareMatrixTag(_assembly, _var.number(), _var.number());
  else
    prepareMatrixTagNeighbor(_assembly, _var.number(), _var.number(), type);

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    precalculateQpJacobian(type);
    for (_i = 0; _i < test_space.size(); _i++)
      for (_j = 0; _j < loc_phi.size(); _j++)
        _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpJacobian(type);
  }

  accumulateTaggedLocalMatrix();

  if (_has_diag_save_in && (type == Moose::ElementElement || type == Moose::NeighborNeighbor))
  {
    unsigned int rows = _local_ke.m();
    DenseVector<Number> diag(rows);
    for (unsigned int i = 0; i < rows; i++)
      diag(i) = _local_ke(i, i);

    Threads::spin_mutex::scoped_lock lock(_jacoby_vars_mutex);
    for (const auto & var : _diag_save_in)
    {
      if (type == Moose::ElementElement)
        var->sys().solution().add_vector(diag, var->dofIndices());
      else
        var->sys().solution().add_vector(diag, var->dofIndicesNeighbor());
    }
  }
}

void
DGKernel::computeOffDiagElemNeighJacobian(Moose::DGJacobianType type,
                                          const MooseVariableFEBase & jvar)
{
  const VariableTestValue & test_space =
      (type == Moose::ElementElement || type == Moose::ElementNeighbor) ? _test : _test_neighbor;
  const VariableTestValue & loc_phi =
      (type == Moose::ElementElement || type == Moose::NeighborElement) ? _phi : _phi_neighbor;

  if (type == Moose::ElementElement)
    prepareMatrixTag(_assembly, _var.number(), jvar.number());
  else
    prepareMatrixTagNeighbor(_assembly, _var.number(), jvar.number(), type);

  if (_local_ke.n() == 0 || _local_ke.m() == 0)
    return;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    precalculateQpOffDiagJacobian(type, jvar);
    for (_i = 0; _i < test_space.size(); _i++)
      for (_j = 0; _j < loc_phi.size(); _j++)
        _local_ke(_i, _j) +=
            _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(type, jvar.number());
  }

  accumulateTaggedLocalMatrix();
}

Real
DGKernel::computeQpOffDiagJacobian(Moose::DGJacobianType /*type*/, unsigned int /*jvar*/)
{
  return 0.;
}
