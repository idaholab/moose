//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADKernelValue.h"

// libmesh includes
#include "libmesh/threads.h"

defineADValidParams(ADKernelValue, ADKernel, );
defineADValidParams(ADVectorKernelValue, ADVectorKernel, );

template <typename T, ComputeStage compute_stage>
ADKernelValueTempl<T, compute_stage>::ADKernelValueTempl(const InputParameters & parameters)
  : ADKernelTempl<T, compute_stage>(parameters)
{
}

template <typename T, ComputeStage compute_stage>
void
ADKernelValueTempl<T, compute_stage>::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  precalculateResidual();
  const unsigned int n_test = _test.size();
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    const auto value = precomputeQpResidual() * _ad_JxW[_qp] * _ad_coord[_qp];
    for (_i = 0; _i < n_test; _i++) // target for auto vectorization
      _local_re(_i) += value * _test[_i][_qp];
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
ADKernelValueTempl<Real, JACOBIAN>::computeResidual()
{
}

template <>
void
ADKernelValueTempl<RealVectorValue, JACOBIAN>::computeResidual()
{
}

template <typename T, ComputeStage compute_stage>
void
ADKernelValueTempl<T, compute_stage>::computeJacobian()
{
  prepareMatrixTag(_assembly, _var.number(), _var.number());

  size_t ad_offset = _var.number() * _sys.getMaxVarNDofsPerElem();

  precalculateResidual();
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    // This will also compute the derivative with respect to all dofs
    const auto value = precomputeQpResidual() * _ad_JxW[_qp] * _ad_coord[_qp];
    for (_i = 0; _i < _test.size(); _i++)
    {
      const auto residual = value * _test[_i][_qp];
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
ADKernelValueTempl<Real, RESIDUAL>::computeJacobian()
{
}

template <>
void
ADKernelValueTempl<RealVectorValue, RESIDUAL>::computeJacobian()
{
}

template <typename T, ComputeStage compute_stage>
void
ADKernelValueTempl<T, compute_stage>::computeADOffDiagJacobian()
{
  std::vector<DualReal> residuals(_test.size(), 0);

  precalculateResidual();
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    const auto value = precomputeQpResidual() * _ad_JxW[_qp] * _ad_coord[_qp];
    for (_i = 0; _i < _test.size(); _i++)
      residuals[_i] += value * _test[_i][_qp];
  }

  std::vector<std::pair<MooseVariableFEBase *, MooseVariableFEBase *>> & ce =
      _assembly.couplingEntries();
  for (const auto & it : ce)
  {
    MooseVariableFEBase & ivariable = *(it.first);
    MooseVariableFEBase & jvariable = *(it.second);

    unsigned int ivar = ivariable.number();
    unsigned int jvar = jvariable.number();

    if (ivar != _var.number())
      continue;

    size_t ad_offset = jvar * _sys.getMaxVarNDofsPerElem();

    prepareMatrixTag(_assembly, ivar, jvar);

    if (_local_ke.m() != _test.size() || _local_ke.n() != jvariable.phiSize())
      continue;

    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < jvariable.phiSize(); _j++)
        _local_ke(_i, _j) += residuals[_i].derivatives()[ad_offset + _j];

    accumulateTaggedLocalMatrix();
  }
}

template <>
void
ADKernelValueTempl<Real, RESIDUAL>::computeADOffDiagJacobian()
{
}

template <>
void
ADKernelValueTempl<RealVectorValue, RESIDUAL>::computeADOffDiagJacobian()
{
}

template <typename T, ComputeStage compute_stage>
ADResidual
ADKernelValueTempl<T, compute_stage>::computeQpResidual()
{
  mooseError("Override precomputeQpResidual() in your ADKernelValueTempl derived class!");
}

template class ADKernelValueTempl<Real, RESIDUAL>;
template class ADKernelValueTempl<Real, JACOBIAN>;
template class ADKernelValueTempl<RealVectorValue, RESIDUAL>;
template class ADKernelValueTempl<RealVectorValue, JACOBIAN>;
