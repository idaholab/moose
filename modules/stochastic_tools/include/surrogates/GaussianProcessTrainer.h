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

#include "GaussianProcessHandler.h"

class GaussianProcessTrainer : public SurrogateTrainer, public CovarianceInterface
{
public:
  static InputParameters validParams();
  GaussianProcessTrainer(const InputParameters & parameters);
  virtual void preTrain() override;
  virtual void train() override;
  virtual void postTrain() override;

  StochasticTools::GaussianProcessHandler & gpHandler() { return _gp_handler; }
  const StochasticTools::GaussianProcessHandler & getGPHandler() const { return _gp_handler; }

private:
  /// Data from the current predictor row
  const std::vector<Real> & _predictor_row;

  StochasticTools::GaussianProcessHandler & _gp_handler;

  /// Parameters (x) used for training -- we'll allgather these in postTrain().
  std::vector<std::vector<Real>> _params_buffer;

  /// Data (y) used for training.
  std::vector<Real> _data_buffer;

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

  /// Enum which contains the hyper parameter optimizaton type requested by the user
  MooseEnum _tuning_algorithm;

  /*
  /// Response value
  const Real & _rval;

  /// Predictor values from reporters
  std::vector<const Real *> _pvals;

  /// Columns from sampler for predictors
  std::vector<unsigned int> _pcols;

  /// Total number of parameters/dimensions
  unsigned int _n_params;
  */
};
