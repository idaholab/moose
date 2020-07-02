//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CovarianceFunctionBase.h"

// InputParameters
// CovarianceFunctionBase::validParams()
// {
//   InputParameters params = MooseObject::validParams();
//   params.addRequiredParam<std::vector<Real>>("length_factor",
//                                              "Length Factor to use for Covariance Kernel");
//   params.addParam<Real>(
//       "signal_variance", 1, "Signal Variance (sigma_f^2) to use for kernel calculation.");
//   params.addParam<Real>(
//       "noise_variance", 0, "Noise Variance (sigma_n^2) to use for kernel calculation.");
//   return params;
// }
//
// CovarianceFunctionBase::CovarianceFunctionBase(const InputParameters & parameters)
//   : MooseObject(parameters),
//   _length_factor(getParam<std::vector<Real>>("length_factor")),
//   _signal_variance(getParam<Real>("signal_variance")),
//   _noise_variance(getParam<Real>("noise_variance"))
//
// {
// }

CovarianceFunctionBase::CovarianceFunctionBase(const std::vector<Real> & length_factor,
                                               const Real & sigma_f_squared,
                                               const Real & sigma_n_squared)
  : _sigma_f_squared(sigma_f_squared),
    _sigma_n_squared(sigma_n_squared),
    _length_factor(length_factor)
{
}
