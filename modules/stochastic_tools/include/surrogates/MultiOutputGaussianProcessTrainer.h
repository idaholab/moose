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

#include "OutputCovarianceBase.h"
#include "OutputCovarianceInterface.h"

#include "MultiOutputGaussianProcessHandler.h"

class MultiOutputGaussianProcessTrainer : public SurrogateTrainer,
                                          public CovarianceInterface,
                                          public OutputCovarianceInterface
{
public:
  static InputParameters validParams();
  MultiOutputGaussianProcessTrainer(const InputParameters & parameters);
  virtual void preTrain() override;
  virtual void train() override;
  virtual void postTrain() override;

  // StochasticTools::MultiOutputGaussianProcessHandler & mogpHandler() { return _mogp_handler; }
  // const StochasticTools::MultiOutputGaussianProcessHandler & getmoGPHandler() const
  // {
  //   return _mogp_handler;
  // }

private:
  // /// Data from the current predictor row
  const std::vector<Real> & _predictor_row;

  /// Gaussian process handler responsible for managing training related tasks
  // StochasticTools::MultiOutputGaussianProcessHandler & _mogp_handler;

  /// Parameters (x) used for training -- we'll allgather these in postTrain().
  std::vector<std::vector<Real>> _params_buffer;

  /// Data (y) used for training.
  std::vector<std::vector<Real>> _data_buffer;

  // /// Paramaters (x) used for training, along with statistics
  RealEigenMatrix & _training_params;

  // /// Data (y) used for training
  RealEigenMatrix _training_data;

  /// Struct holding parameters necessary for parameter tuning
  const StochasticTools::MultiOutputGaussianProcessHandler::GPOptimizerOptions _optimization_opts;

  // /// Data from the current sampler row
  const std::vector<Real> & _sampler_row;

  // /// Predictor values from reporters
  std::vector<const Real *> _pvals;

  // /// Columns from sampler for predictors
  std::vector<unsigned int> _pcols;

  // /// Total number of parameters/dimensions
  unsigned int _n_params;
};
