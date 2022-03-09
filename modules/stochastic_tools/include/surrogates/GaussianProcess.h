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
#include "GaussianProcessUtils.h"

class GaussianProcess : public SurrogateModel, public CovarianceInterface
{
public:
  static InputParameters validParams();
  GaussianProcess(const InputParameters & parameters);
  using SurrogateModel::evaluate;
  virtual Real evaluate(const std::vector<Real> & x) const override;
  virtual Real evaluate(const std::vector<Real> & x, Real & std) const override;

  /**
   * This function is called by LoadCovarianceDataAction when the surrogate is
   * loading training data from a file. The action must recreate the covariance
   * object before this surrogate can set the correct pointer.
   */
  virtual void setupCovariance(UserObjectName _covar_name);

  const std::string & getCovarType() const { return _gp_utils.getCovarType(); }

  const std::unordered_map<std::string, Real> & getHyperParamMap() const { return _hyperparam_map; }

  const std::unordered_map<std::string, std::vector<Real>> & getHyperParamVecMap() const
  {
    return _hyperparam_vec_map;
  }

private:

  StochasticTools::GaussianProcessUtils & _gp_utils;

  /// Paramaters (x) used for training
  const RealEigenMatrix & _training_params;

  /// Scalar hyperparameters. Stored for use in surrogate
  const std::unordered_map<std::string, Real> & _hyperparam_map;

  /// Vector hyperparameters. Stored for use in surrogate
  const std::unordered_map<std::string, std::vector<Real>> & _hyperparam_vec_map;

};
