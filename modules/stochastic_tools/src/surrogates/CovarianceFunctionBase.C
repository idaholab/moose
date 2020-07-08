//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CovarianceFunctionBase.h"

InputParameters
CovarianceFunctionBase::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.addParam<std::vector<std::vector<Real>>>(
      "hyperparams", "hyperparams to use in loading covariacne function");
  params.addParam<std::vector<Real>>("length_factor", "Length Factor to use for Covariance Kernel");
  params.addParam<Real>(
      "signal_variance", 1, "Signal Variance (sigma_f^2) to use for kernel calculation.");
  params.addParam<Real>(
      "noise_variance", 0, "Noise Variance (sigma_n^2) to use for kernel calculation.");
  params.registerBase("CovarianceFunctionBase");
  params.registerSystemAttributeName("CovarianceFunctionBase");
  return params;
}

CovarianceFunctionBase::CovarianceFunctionBase(const InputParameters & parameters)
  : MooseObject(parameters),
    _hyperparams(getParam<std::vector<std::vector<Real>>>("hyperparams")),
    _length_factor(!_hyperparams.empty() ? _hyperparams[0]
                                         : getParam<std::vector<Real>>("length_factor")),
    _sigma_f_squared(!_hyperparams.empty() ? _hyperparams[1][0]
                                           : getParam<Real>("signal_variance")),
    _sigma_n_squared(!_hyperparams.empty() ? _hyperparams[2][0] : getParam<Real>("noise_variance"))

{
}

// CovarianceFunctionBase::CovarianceFunctionBase(const std::vector<Real> & length_factor,
//                                                const Real & sigma_f_squared,
//                                                const Real & sigma_n_squared)
//   : _sigma_f_squared(sigma_f_squared),
//     _sigma_n_squared(sigma_n_squared),
//     _length_factor(length_factor)
// {
// }
