//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADKernelGrad.h"

// libmesh includes
#include "libmesh/threads.h"

defineADValidParams(ADKernelGrad, ADKernel, );

template <ComputeStage compute_stage>
ADKernelGrad<compute_stage>::ADKernelGrad(const InputParameters & parameters)
  : ADKernel<compute_stage>(parameters)
{
}

template <ComputeStage compute_stage>
void
ADKernelGrad<compute_stage>::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  precalculateResidual();
  const unsigned int n_test = _grad_test.size();
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    const auto value = precomputeQpResidual() * _ad_JxW[_qp] * _ad_coord[_qp];
    for (_i = 0; _i < n_test; _i++) // target for auto vectorization
      _local_re(_i) += value * _grad_test[_i][_qp];
  }

  accumulateTaggedLocalResidual();

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i = 0; i < _save_in.size(); i++)
      _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());
  }
}

template <>
void
ADKernelGrad<JACOBIAN>::computeResidual()
{
}

template <ComputeStage compute_stage>
void
ADKernelGrad<compute_stage>::computeJacobian()
{
  prepareMatrixTag(_assembly, _var.number(), _var.number());

  size_t ad_offset = _var.number() * _sys.getMaxVarNDofsPerElem();

  precalculateResidual();
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    // This will also compute the derivative with respect to all dofs
    const auto value = precomputeQpResidual() * _ad_JxW[_qp] * _ad_coord[_qp];
    for (_i = 0; _i < _grad_test.size(); _i++)
    {
      const auto residual = value * _grad_test[_i][_qp];
      for (_j = 0; _j < _var.phiSize(); _j++)
        _local_ke(_i, _j) += residual.derivatives()[ad_offset + _j];
    }
  }
  accumulateTaggedLocalMatrix();

  if (_has_diag_save_in)
  {
    unsigned int rows = _local_ke.m();
    DenseVector<Number> diag(rows);
    for (unsigned int i = 0; i < rows; i++)
      diag(i) = _local_ke(i, i);

    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i = 0; i < _diag_save_in.size(); i++)
      _diag_save_in[i]->sys().solution().add_vector(diag, _diag_save_in[i]->dofIndices());
  }
}

template <>
void
ADKernelGrad<RESIDUAL>::computeJacobian()
{
}

template <ComputeStage compute_stage>
void
ADKernelGrad<compute_stage>::computeOffDiagJacobian(MooseVariableFEBase & jvar)
{
  auto jvar_num = jvar.number();

  if (jvar_num == _var.number())
    computeJacobian();
  else
  {
    size_t ad_offset = jvar_num * _sys.getMaxVarNDofsPerElem();

    prepareMatrixTag(_assembly, _var.number(), jvar_num);

    if (_local_ke.m() != _grad_test.size() || _local_ke.n() != jvar.phiSize())
      return;

    precalculateResidual();
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      // This will also compute the derivative with respect to all dofs
      const auto value = precomputeQpResidual() * _ad_JxW[_qp] * _ad_coord[_qp];
      for (_i = 0; _i < _grad_test.size(); _i++)
      {
        const auto residual = value * _grad_test[_i][_qp];
        for (_j = 0; _j < jvar.phiSize(); _j++)
          _local_ke(_i, _j) += residual.derivatives()[ad_offset + _j];
      }
    }

    accumulateTaggedLocalMatrix();
  }
}

template <>
void
ADKernelGrad<RESIDUAL>::computeOffDiagJacobian(MooseVariableFEBase &)
{
}

template <ComputeStage compute_stage>
ADResidual
ADKernelGrad<compute_stage>::computeQpResidual()
{
  mooseError("Override precomputeQpResidual() in your ADKernelGrad derived class!");
}

// explicit instantiation is required for AD base classes
adBaseClass(ADKernelGrad);
