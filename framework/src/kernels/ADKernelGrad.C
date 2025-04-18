//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADKernelGrad.h"
#include "MathUtils.h"
#include "Assembly.h"
#include "SystemBase.h"
#include "ADUtils.h"

// libmesh includes
#include "libmesh/threads.h"

template <typename T>
InputParameters
ADKernelGradTempl<T>::validParams()
{
  return ADKernelTempl<T>::validParams();
}

template <typename T>
ADKernelGradTempl<T>::ADKernelGradTempl(const InputParameters & parameters)
  : ADKernelTempl<T>(parameters)
{
}

template <typename T>
void
ADKernelGradTempl<T>::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  precalculateResidual();
  const unsigned int n_test = _grad_test.size();
  std::vector<Real> residuals(n_test);

  if (_use_displaced_mesh)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      const auto value = precomputeQpResidual() * _ad_JxW[_qp] * _ad_coord[_qp];
      for (_i = 0; _i < n_test; _i++) // target for auto vectorization
        residuals[_i] += raw_value(MathUtils::dotProduct(value, _grad_test[_i][_qp]));
    }
  else
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      const auto value = precomputeQpResidual() * _JxW[_qp] * _coord[_qp];
      for (_i = 0; _i < n_test; _i++) // target for auto vectorization
        residuals[_i] += raw_value(MathUtils::dotProduct(value, _regular_grad_test[_i][_qp]));
    }

  this->addResiduals(_assembly, residuals, _var.dofIndices(), _var.scalingFactor());

  if (_has_save_in)
    for (unsigned int i = 0; i < _save_in.size(); i++)
      _save_in[i]->sys().solution().add_vector(residuals.data(), _save_in[i]->dofIndices());
}

template <typename T>
void
ADKernelGradTempl<T>::computeResidualsForJacobian()
{
  if (_residuals.size() != _grad_test.size())
    _residuals.resize(_grad_test.size(), 0);
  for (auto & r : _residuals)
    r = 0;

  precalculateResidual();
  if (_use_displaced_mesh)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      const auto value = precomputeQpResidual() * _ad_JxW[_qp] * _ad_coord[_qp];
      for (_i = 0; _i < _grad_test.size(); _i++)
        _residuals[_i] += MathUtils::dotProduct(value, _grad_test[_i][_qp]);
    }
  else
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      const auto value = precomputeQpResidual() * _JxW[_qp] * _coord[_qp];
      for (_i = 0; _i < _grad_test.size(); _i++)
        _residuals[_i] += MathUtils::dotProduct(value, _regular_grad_test[_i][_qp]);
    }
}

template <typename T>
void
ADKernelGradTempl<T>::computeResidualAndJacobian()
{
  computeResidualsForJacobian();
  this->addResidualsAndJacobian(_assembly, _residuals, _var.dofIndices(), _var.scalingFactor());
}

template <typename T>
ADReal
ADKernelGradTempl<T>::computeQpResidual()
{
  mooseError("Override precomputeQpResidual() in your ADKernelGrad derived class!");
}

template class ADKernelGradTempl<Real>;
template class ADKernelGradTempl<RealVectorValue>;
