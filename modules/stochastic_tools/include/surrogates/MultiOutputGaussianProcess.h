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
#include "MultiOutputGaussianProcessTrainer.h"
#include "Standardizer.h"
#include <Eigen/Dense>
#include "CovarianceInterface.h"
#include "OutputCovarianceInterface.h"
#include "MultiOutputGaussianProcessHandler.h"

class MultiOutputGaussianProcess : public SurrogateModel,
                                   public CovarianceInterface,
                                   public OutputCovarianceInterface
{
public:
  static InputParameters validParams();
  MultiOutputGaussianProcess(const InputParameters & parameters);
  using SurrogateModel::evaluate;
  virtual void evaluate(const std::vector<Real> & x, std::vector<Real> & y) const override;

  // /**
  //  * This function is called by LoadCovarianceDataAction when the surrogate is
  //  * loading training data from a file. The action must recreate the covariance
  //  * object before this surrogate can set the correct pointer.
  //  */
  // virtual void setupCovariance(UserObjectName _covar_name);

  StochasticTools::MultiOutputGaussianProcessHandler & mogpHandler() { return _mogp_handler; }
  const StochasticTools::MultiOutputGaussianProcessHandler & getmoGPHandler() const
  {
    return _mogp_handler;
  }

private:
  StochasticTools::MultiOutputGaussianProcessHandler & _mogp_handler;

  /// Paramaters (x) used for training
  const RealEigenMatrix & _training_params;
};
