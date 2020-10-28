//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADKernelValue.h"
#include "Assembly.h"
#include "SystemBase.h"
#include "ADUtils.h"

// libmesh includes
#include "libmesh/threads.h"

template <typename T>
InputParameters
ADKernelValueTempl<T>::validParams()
{
  return ADKernelTempl<T>::validParams();
}

template <typename T>
ADKernelValueTempl<T>::ADKernelValueTempl(const InputParameters & parameters)
  : ADKernelTempl<T>(parameters)
{
}

template <typename T>
void
ADKernelValueTempl<T>::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  precalculateResidual();
  const unsigned int n_test = _test.size();

  if (_use_displaced_mesh)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      const auto value = precomputeQpResidual() * _ad_JxW[_qp] * _ad_coord[_qp];
      for (_i = 0; _i < n_test; _i++) // target for auto vectorization
        _local_re(_i) += raw_value(value * _test[_i][_qp]);
    }
  else
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      const auto value = precomputeQpResidual() * _JxW[_qp] * _coord[_qp];
      for (_i = 0; _i < n_test; _i++) // target for auto vectorization
        _local_re(_i) += raw_value(value * _test[_i][_qp]);
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
ADKernelValueTempl<T>::computeResidualsForJacobian()
{
  if (_residuals.size() != _test.size())
    _residuals.resize(_test.size(), 0);
  for (auto & r : _residuals)
    r = 0;

  precalculateResidual();

  if (_use_displaced_mesh)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      const auto value = precomputeQpResidual() * _ad_JxW[_qp] * _ad_coord[_qp];
      for (_i = 0; _i < _test.size(); _i++)
        _residuals[_i] += value * _test[_i][_qp];
    }
  else
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      const auto value = precomputeQpResidual() * _JxW[_qp] * _coord[_qp];
      for (_i = 0; _i < _test.size(); _i++)
        _residuals[_i] += value * _test[_i][_qp];
    }
}

template <typename T>
ADReal
ADKernelValueTempl<T>::computeQpResidual()
{
  mooseError("Override precomputeQpResidual() in your ADKernelValueTempl derived class!");
}

template class ADKernelValueTempl<Real>;
template class ADKernelValueTempl<RealVectorValue>;
