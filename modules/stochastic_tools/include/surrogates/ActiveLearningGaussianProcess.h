//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ActiveLearningGaussianProcess.h"
#include "Standardizer.h"
#include <Eigen/Dense>

#include "StochasticToolsApp.h"
#include "LoadSurrogateDataAction.h"

#include "SurrogateModelInterface.h"
#include "SurrogateTrainer.h"
#include "MooseRandom.h"

#include "Distribution.h"

#include "CovarianceFunctionBase.h"
#include "CovarianceInterface.h"

#include "GaussianProcess.h"

class ActiveLearningGaussianProcess : public SurrogateTrainerBase,
                                      public CovarianceInterface,
                                      public SurrogateModelInterface
{
public:
  static InputParameters validParams();
  ActiveLearningGaussianProcess(const InputParameters & parameters);

  virtual void initialize() final {}
  virtual void execute() final {}
  virtual void reTrain(const std::vector<std::vector<Real>> & inputs,
                       const std::vector<Real> & outputs) const final;

  StochasticTools::GaussianProcess & gp() { return _gp; }
  const StochasticTools::GaussianProcess & getGP() const { return _gp; }

private:
  /// Name for the meta data associated with training
  const std::string _model_meta_data_name;

  /// The GP handler
  StochasticTools::GaussianProcess & _gp;

  /// Paramaters (x) used for training, along with statistics
  RealEigenMatrix & _training_params;

  /// Switch for training param (x) standardization
  bool _standardize_params;

  /// Switch for training data(y) standardization
  bool _standardize_data;

  /// Struct holding parameters necessary for parameter tuning
  const StochasticTools::GaussianProcess::GPOptimizerOptions _optimization_opts;
};
