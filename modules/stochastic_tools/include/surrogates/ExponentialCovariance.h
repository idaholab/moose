//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CovarianceFunctionBase.h"

class ExponentialCovariance : public CovarianceFunctionBase
{
public:
  static InputParameters validParams();
  ExponentialCovariance(const InputParameters & parameters);

  /// Generates the Covariance Matrix given two points in the parameter space
  void computeCovarianceMatrix(RealEigenMatrix & K,
                               const RealEigenMatrix & x,
                               const RealEigenMatrix & xp,
                               const bool is_self_covariance) const override;

  static void ExponentialFunction(RealEigenMatrix & K,
                                  const RealEigenMatrix & x,
                                  const RealEigenMatrix & xp,
                                  const std::vector<Real> & length_factor,
                                  const Real sigma_f_squared,
                                  const Real sigma_n_squared,
                                  const Real gamma,
                                  const bool is_self_covariance);

  /// Used for outputting Hyper-parameter settings
  void
  buildHyperParamMap(std::unordered_map<std::string, Real> & map,
                     std::unordered_map<std::string, std::vector<Real>> & vec_map) const override;

private:
  /// lengh factor (\ell) for the kernel, in vector form for multiple parameters
  std::vector<Real> _length_factor;

  /// signal variance (\sigma_f^2)
  Real _sigma_f_squared;

  /// noise variance (\sigma_n^2)
  Real _sigma_n_squared;

  /// gamma exponential factor for use in kernel
  Real _gamma;
};
