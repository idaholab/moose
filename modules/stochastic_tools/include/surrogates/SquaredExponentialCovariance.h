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

class SquaredExponentialCovariance : public CovarianceFunctionBase
{
public:
  static InputParameters validParams();
  SquaredExponentialCovariance(const InputParameters & parameters);

  // SquaredExponentialCovariance(const std::vector<Real> & length_factor,
  //                                    const Real & sigma_f_squared,
  //                                    const Real & sigma_n_squared);
  //
  // SquaredExponentialCovariance(const std::vector<std::vector<Real>> & vec);

  /// Generates the Covariance Matrix given two points in the parameter space
  RealEigenMatrix computeCovarianceMatrix(const RealEigenMatrix & x,
                                          const RealEigenMatrix & xp,
                                          const bool is_self_covariance) const override;

  /// Used for outputting Hyper-parameter settings
  void
  buildHyperParamMap(std::unordered_map<std::string, Real> & map,
                     std::unordered_map<std::string, std::vector<Real>> & vec_map) const override;

protected:
  /// lengh factor (\ell) for the kernel, in vector form for multiple parameters
  std::vector<Real> _length_factor;

  /// signal variance (\sigma_f^2)
  Real _sigma_f_squared;

  /// noise variance (\sigma_n^2)
  Real _sigma_n_squared;
};
