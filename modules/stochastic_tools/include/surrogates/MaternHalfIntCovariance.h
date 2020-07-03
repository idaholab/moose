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

class MaternHalfIntCovariance : public CovarianceFunctionBase
{
public:
  static InputParameters validParams();
  MaternHalfIntCovariance(const InputParameters & parameters);

  // MaternHalfIntCovariance(const std::vector<Real> & length_factor,
  //                               const Real & sigma_f_squared,
  //                               const Real & sigma_n_squared,
  //                               const unsigned int & p);
  //
  // MaternHalfIntCovariance(const std::vector<std::vector<Real>> & vec);

  /// Generates the Covariance Matrix given two points in the parameter space
  RealEigenMatrix computeCovarianceMatrix(const RealEigenMatrix & x,
                                          const RealEigenMatrix & xp,
                                          const bool is_self_covariance) const override;

  /// Used for outputting Hyper-parameter settings
  void getHyperParameters(std::vector<std::vector<Real>> & vec) const override;

private:
  /// non-negative p factor for use in Matern half-int. \nu = p+(1/2) in terms of general Matern
  unsigned int _p;
};
