//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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

#include "GaussianProcessTorched.h"

#include "LibtorchUtils.h"

class GaussianProcessTrainerTorched : public SurrogateTrainer, public CovarianceInterface
{
public:
  static InputParameters validParams();
  GaussianProcessTrainerTorched(const InputParameters & parameters);
  virtual void preTrain() override;
  virtual void train() override;
  virtual void postTrain() override;

  StochasticToolsTorched::GaussianProcessTorched & gp() { return _gp; }
  const StochasticToolsTorched::GaussianProcessTorched & gp() const { return _gp; }

private:
  /// Data from the current predictor row
  const std::vector<Real> & _predictor_row;

  /// Gaussian process handler responsible for managing training related tasks
  StochasticToolsTorched::GaussianProcessTorched & _gp;

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
  const StochasticToolsTorched::GaussianProcessTorched::GPOptimizerOptions _optimization_opts;

  /// Data from the current sampler row
  const std::vector<Real> & _sampler_row;
};
