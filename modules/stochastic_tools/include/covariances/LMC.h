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
#include "CovarianceInterface.h"

class LMC : public CovarianceFunctionBase, public CovarianceInterface
{
public:
  static InputParameters validParams();
  LMC(const InputParameters & parameters);

  //// Generates the Covariance Matrix given two points in the parameter space
  void computeCovarianceMatrix(RealEigenMatrix & K,
                               const RealEigenMatrix & x,
                               const RealEigenMatrix & xp,
                               const bool is_self_covariance) const override;

  /// Redirect dK/dhp for hyperparameter "hp"
  void computedKdhyper(RealEigenMatrix & dKdhp,
                       const RealEigenMatrix & x,
                       std::string hyper_param_name,
                       unsigned int ind) const override;

  /// Computes dK/dlf for individual length factors
  static void computedKdlf(RealEigenMatrix & K,
                           const RealEigenMatrix & x,
                           const std::vector<Real> & length_factor,
                           const Real sigma_f_squared,
                           const int ind);

  /// Used for outputting Hyper-parameter settings
  virtual void buildHyperParamMap(
      std::unordered_map<std::string, Real> & /*map*/,
      std::unordered_map<std::string, std::vector<Real>> & /*vec_map*/) const override
  {
  }

  /// Used for outputting Hyper-parameter settings for use in surrogate
  virtual void
  loadHyperParamMap(std::unordered_map<std::string, Real> & /*map*/,
                    std::unordered_map<std::string, std::vector<Real>> & /*vec_map*/) override
  {
  }

  /// Get the default minima, maxima, sizes of a hyper parameter
  virtual void getTuningData(std::string /*name*/,
                             unsigned int & /*size*/,
                             Real & /*min*/,
                             Real & /*max*/) const override
  {
  }

protected:
  const unsigned int _num_expansion_terms;
};
