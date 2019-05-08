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

template <>
InputParameters
validParams<DGKernel>()
{
  InputParameters params = validParams<DGKernelBase>();
  return params;
}

DGKernel::DGKernel(const InputParameters & parameters) : DGKernelBase(parameters) {}

DGKernel::~DGKernel() {}

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
    for (_i = 0; _i < test_space.size(); _i++)
      _local_re(_i) += _JxW[_qp] * _coord[_qp] * computeQpResidual(type);

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
    for (_i = 0; _i < test_space.size(); _i++)
      for (_j = 0; _j < loc_phi.size(); _j++)
        _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpJacobian(type);

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
DGKernel::computeOffDiagElemNeighJacobian(Moose::DGJacobianType type, unsigned int jvar)
{
  const VariableTestValue & test_space =
      (type == Moose::ElementElement || type == Moose::ElementNeighbor) ? _test : _test_neighbor;
  const VariableTestValue & loc_phi =
      (type == Moose::ElementElement || type == Moose::NeighborElement) ? _phi : _phi_neighbor;

  if (type == Moose::ElementElement)
    prepareMatrixTag(_assembly, _var.number(), jvar);
  else
    prepareMatrixTagNeighbor(_assembly, _var.number(), jvar, type);

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < test_space.size(); _i++)
      for (_j = 0; _j < loc_phi.size(); _j++)
        _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(type, jvar);

  accumulateTaggedLocalMatrix();
}

Real
DGKernel::computeQpOffDiagJacobian(Moose::DGJacobianType /*type*/, unsigned int /*jvar*/)
{
  return 0.;
}
