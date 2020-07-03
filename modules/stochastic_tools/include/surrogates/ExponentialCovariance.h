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

  // ExponentialCovariance(const std::vector<Real> & length_factor,
  //                             const Real & sigma_f_squared,
  //                             const Real & sigma_n_squared,
  //                             const Real gamma);
  //
  // ExponentialCovariance(const std::vector<std::vector<Real>> & vec);

  /// Generates the Covariance Matrix given two points in the parameter space
  RealEigenMatrix computeCovarianceMatrix(const RealEigenMatrix & x,
                                          const RealEigenMatrix & xp,
                                          const bool is_self_covariance) const override;

  /// Used for outputting Hyper-parameter settings
  void getHyperParameters(std::vector<std::vector<Real>> & vec) const override;

private:
  /// gamma exponential factor for use in kernel
  Real _gamma;
};
