//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "OutputCovarianceBase.h"

class IntrinsicCoregionalizationModel : public OutputCovarianceBase
{
public:
  static InputParameters validParams();
  IntrinsicCoregionalizationModel(const InputParameters & parameters);

  /// Generates the B Covariance Matrix for capturing output covariances
  void computeBCovarianceMatrix(RealEigenMatrix & B, const std::vector<Real> & latent) const override;

  /// Generates the full Covariance Matrix given two points in the parameter space
  void computeFullCovarianceMatrix(RealEigenMatrix & kappa,
                                   const RealEigenMatrix & B,
                                   const RealEigenMatrix & K,
                                   const RealEigenMatrix & x,
                                   const RealEigenMatrix & xp,
                                   const bool is_self_covariance) const override;

  /// Compute the gradient of the B matrix
  void computeBGrad(RealEigenMatrix & BGrad, const std::vector<Real> & latent) const override;

  /// Setup the number of latent params
  unsigned int setupNumLatent(const unsigned int & num_outputs) const override;

private:
  /// gamma exponential factor for use in kernel
  // Real _gamma;
};
