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
#include "GaussianProcessHandler.h"

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

  StochasticTools::GaussianProcessHandler & gpHandler() { return _gp_handler; }
  const StochasticTools::GaussianProcessHandler & getGPHandler() const { return _gp_handler; }

private:
  StochasticTools::GaussianProcessHandler & _gp_handler;

  /// Paramaters (x) used for training
  const RealEigenMatrix & _training_params;
};
