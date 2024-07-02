//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SurrogateTrainer.h"
#include "Standardizer.h"
#include <Eigen/Dense>

#include "Distribution.h"

#include "CovarianceFunctionBase.h"
#include "CovarianceInterface.h"

#include "GaussianProcess.h"

class GaussianProcessTrainer : public SurrogateTrainer, public CovarianceInterface
{
public:
  static InputParameters validParams();
  GaussianProcessTrainer(const InputParameters & parameters);
  virtual void preTrain() override;
  virtual void train() override;
  virtual void postTrain() override;

  StochasticTools::GaussianProcess & gp() { return _gp; }
  const StochasticTools::GaussianProcess & gp() const { return _gp; }

private:
  /// Data from the current predictor row
  const std::vector<Real> & _predictor_row;

  /// Gaussian process handler responsible for managing training related tasks
  StochasticTools::GaussianProcess & _gp;

  /// Parameters (x) used for training -- we'll allgather these in postTrain().
  std::vector<std::vector<Real>> _params_buffer;

  /// Data (y) used for training.
  std::vector<std::vector<Real>> _data_buffer;

  /// Paramaters (x) used for training, along with statistics
  RealEigenMatrix & _training_params;

  /// Data (y) used for training
  RealEigenMatrix _training_data;

  /// Switch for training param (x) standardization
  bool _standardize_params;

  /// Switch for training data(y) standardization
  bool _standardize_data;

  /// Flag to toggle hyperparameter tuning/optimization
  bool _do_tuning;

  /// Struct holding parameters necessary for parameter tuning
  const StochasticTools::GaussianProcess::GPOptimizerOptions _optimization_opts;

  /// Data from the current sampler row
  const std::vector<Real> & _sampler_row;
};
