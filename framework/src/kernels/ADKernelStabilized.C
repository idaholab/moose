//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADKernelStabilized.h"
#include "MathUtils.h"
#include "Assembly.h"
#include "SystemBase.h"
#include "ADUtils.h"

// libmesh includes
#include "libmesh/threads.h"

template <typename T>
InputParameters
ADKernelStabilizedTempl<T>::validParams()
{
  return ADKernelTempl<T>::validParams();
}

template <typename T>
ADKernelStabilizedTempl<T>::ADKernelStabilizedTempl(const InputParameters & parameters)
  : ADKernelTempl<T>(parameters)
{
}

template <typename T>
void
ADKernelStabilizedTempl<T>::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  precalculateResidual();
  const unsigned int n_test = _grad_test.size();

  if (_use_displaced_mesh)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      const auto value = precomputeQpStrongResidual() * _ad_JxW[_qp] * _ad_coord[_qp];
      for (_i = 0; _i < n_test; _i++) // target for auto vectorization
        _local_re(_i) += raw_value(_grad_test[_i][_qp] * computeQpStabilization() * value);
    }
  else
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      const auto value = precomputeQpStrongResidual() * _JxW[_qp] * _coord[_qp];
      for (_i = 0; _i < n_test; _i++) // target for auto vectorization
        _local_re(_i) += raw_value(_regular_grad_test[_i][_qp] * computeQpStabilization() * value);
    }

  accumulateTaggedLocalResidual();

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (unsigned int i = 0; i < _save_in.size(); i++)
      _save_in[i]->sys().solution().add_vector(_local_re, _save_in[i]->dofIndices());
  }
}

template <typename T>
void
ADKernelStabilizedTempl<T>::computeResidualsForJacobian()
{
  if (_residuals.size() != _grad_test.size())
    _residuals.resize(_grad_test.size(), 0);
  for (auto & r : _residuals)
    r = 0;

  precalculateResidual();

  if (_use_displaced_mesh)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      const auto value = precomputeQpStrongResidual() * _ad_JxW[_qp] * _ad_coord[_qp];
      for (_i = 0; _i < _grad_test.size(); _i++)
        _residuals[_i] += _grad_test[_i][_qp] * computeQpStabilization() * value;
    }
  else
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      const auto value = precomputeQpStrongResidual() * _JxW[_qp] * _coord[_qp];
      for (_i = 0; _i < _grad_test.size(); _i++)
        _residuals[_i] += _regular_grad_test[_i][_qp] * computeQpStabilization() * value;
    }
}

template <typename T>
ADReal
ADKernelStabilizedTempl<T>::computeQpResidual()
{
  mooseError("Override precomputeQpStrongResidual() in your ADKernelStabilized derived class!");
}

template class ADKernelStabilizedTempl<Real>;
template class ADKernelStabilizedTempl<RealVectorValue>;
