//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SurrogateModel.h"
#include "GaussianProcessTrainer.h"
#include "Standardizer.h"
#include <Eigen/Dense>
#include "CovarianceInterface.h"

class GaussianProcess : public SurrogateModel, public CovarianceInterface
{
public:
  static InputParameters validParams();
  GaussianProcess(const InputParameters & parameters);
  virtual Real evaluate(const std::vector<Real> & x) const override;
  virtual Real evaluate(const std::vector<Real> & x, Real & std) const;

  /**
   * This function is called by LoadCovarianceDataAction when the surrogate is
   * loading training data from a file. The action must recreate the covariance
   * object before this surrogate can set the correct pointer.
   */
  virtual void setupCovariance(UserObjectName _covar_name);

  const std::string & getCovarType() const { return _covar_type; }

  const std::unordered_map<std::string, Real> & getHyperParamMap() const { return _hyperparam_map; }

  const std::unordered_map<std::string, std::vector<Real>> & getHyperParamVecMap() const
  {
    return _hyperparam_vec_map;
  }

private:
  /// Paramaters (x) used for training
  const RealEigenMatrix & _training_params;

  /// Standardizer for use with params (x)
  const StochasticTools::Standardizer & _param_standardizer;

  /// Standardizer for use with data (y)
  const StochasticTools::Standardizer & _data_standardizer;

  /// An _n_sample by _n_sample covariance matrix constructed from the selected kernel function
  const RealEigenMatrix & _K;

  /// A solve of Ax=b via Cholesky.
  const RealEigenMatrix & _K_results_solve;

  /// Type of covariance function used for this surrogate
  const std::string & _covar_type;

  /// Scalar hyperparameters. Stored for use in surrogate
  const std::unordered_map<std::string, Real> & _hyperparam_map;

  /// Vector hyperparameters. Stored for use in surrogate
  const std::unordered_map<std::string, std::vector<Real>> & _hyperparam_vec_map;

  /// Covariance function object
  CovarianceFunctionBase * _covariance_function = nullptr;
};
