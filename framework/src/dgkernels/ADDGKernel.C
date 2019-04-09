//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADDGKernel.h"
#include "Assembly.h"
#include "MooseVariable.h"
#include "Problem.h"
#include "SubProblem.h"
#include "NonlinearSystemBase.h"

// libmesh includes
#include "libmesh/threads.h"

defineADBaseValidParams(ADDGKernel, DGKernelBase, );

template <ComputeStage compute_stage>
ADDGKernel<compute_stage>::ADDGKernel(const InputParameters & parameters)
  : DGKernelBase(parameters),
    _u(_var.adSln<compute_stage>()),
    _grad_u(_var.adGradSln<compute_stage>()),
    _u_neighbor(_var.adSlnNeighbor<compute_stage>()),
    _grad_u_neighbor(_var.adGradSlnNeighbor<compute_stage>())
{
}

template <ComputeStage compute_stage>
ADDGKernel<compute_stage>::~ADDGKernel()
{
}

template <ComputeStage compute_stage>
void
ADDGKernel<compute_stage>::computeResidual()
{
  DGKernelBase::computeResidual();
}

template <>
void
ADDGKernel<JACOBIAN>::computeResidual()
{
}

template <ComputeStage compute_stage>
void
ADDGKernel<compute_stage>::computeElemNeighResidual(Moose::DGResidualType type)
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
      std::vector<dof_id_type> & dof_indices =
          is_elem ? var->dofIndices() : var->dofIndicesNeighbor();
      var->sys().solution().add_vector(_local_re, dof_indices);
    }
  }
}

template <>
void ADDGKernel<JACOBIAN>::computeElemNeighResidual(Moose::DGResidualType /*type*/)
{
}

template <ComputeStage compute_stage>
void
ADDGKernel<compute_stage>::computeJacobian()
{
  DGKernelBase::computeJacobian();
}

template <>
void
ADDGKernel<RESIDUAL>::computeJacobian()
{
}

template <ComputeStage compute_stage>
void
ADDGKernel<compute_stage>::computeElemNeighJacobian(Moose::DGJacobianType type)
{
  const VariableTestValue & test_space =
      (type == Moose::ElementElement || type == Moose::ElementNeighbor) ? _test : _test_neighbor;
  const VariableTestValue & loc_phi =
      (type == Moose::ElementElement || type == Moose::NeighborElement) ? _phi : _phi_neighbor;

  if (type == Moose::ElementElement)
    prepareMatrixTag(_assembly, _var.number(), _var.number());
  else
    prepareMatrixTagNeighbor(_assembly, _var.number(), _var.number(), type);

  size_t ad_offset = 0;
  if (type == Moose::ElementElement || type == Moose::NeighborElement)
    ad_offset = _var.number() * _sys.getMaxVarNDofsPerElem();
  else
    ad_offset = _var.number() * _sys.getMaxVarNDofsPerElem() +
                (_sys.system().n_vars() * _sys.getMaxVarNDofsPerElem());

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < test_space.size(); _i++)
    {
      DualReal residual = computeQpResidual(
          (type == Moose::ElementElement || type == Moose::ElementNeighbor) ? Moose::Element
                                                                            : Moose::Neighbor);
      for (_j = 0; _j < loc_phi.size(); _j++)
        _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * residual.derivatives()[ad_offset + _j];
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

template <ComputeStage compute_stage>
void
ADDGKernel<compute_stage>::computeOffDiagJacobian(unsigned int jvar)
{
  DGKernelBase::computeOffDiagJacobian(jvar);
}

template <>
void
ADDGKernel<RESIDUAL>::computeOffDiagJacobian(unsigned int)
{
}

template <ComputeStage compute_stage>
void
ADDGKernel<compute_stage>::computeOffDiagElemNeighJacobian(Moose::DGJacobianType type,
                                                           unsigned int jvar)
{
  const VariableTestValue & test_space =
      (type == Moose::ElementElement || type == Moose::ElementNeighbor) ? _test : _test_neighbor;
  const VariableTestValue & loc_phi =
      (type == Moose::ElementElement || type == Moose::NeighborElement) ? _phi : _phi_neighbor;

  if (type == Moose::ElementElement)
    prepareMatrixTag(_assembly, _var.number(), jvar);
  else
    prepareMatrixTagNeighbor(_assembly, _var.number(), jvar, type);

  size_t ad_offset = 0;
  if (type == Moose::ElementElement || type == Moose::NeighborElement)
    ad_offset = jvar * _sys.getMaxVarNDofsPerElem();
  else
    ad_offset = jvar * _sys.getMaxVarNDofsPerElem() +
                (_sys.system().n_vars() * _sys.getMaxVarNDofsPerElem());

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < test_space.size(); _i++)
    {
      DualReal residual = computeQpResidual(
          (type == Moose::ElementElement || type == Moose::ElementNeighbor) ? Moose::Element
                                                                            : Moose::Neighbor);
      for (_j = 0; _j < loc_phi.size(); _j++)
        _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * residual.derivatives()[ad_offset + _j];
    }

  accumulateTaggedLocalMatrix();
}

adBaseClass(ADDGKernel);
