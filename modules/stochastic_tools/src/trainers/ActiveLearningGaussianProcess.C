//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ActiveLearningGaussianProcess.h"

#include <petsctao.h>
#include <petscdmda.h>

#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"

#include <math.h>

registerMooseObject("StochasticToolsApp", ActiveLearningGaussianProcess);

InputParameters
ActiveLearningGaussianProcess::validParams()
{
  InputParameters params = SurrogateTrainerBase::validParams();
  params.addClassDescription(
      "Permit re-training Gaussian Process surrogate model for active learning.");
  params.addRequiredParam<UserObjectName>("covariance_function", "Name of covariance function.");
  params.addParam<bool>(
      "standardize_params", true, "Standardize (center and scale) training parameters (x values)");
  params.addParam<bool>(
      "standardize_data", true, "Standardize (center and scale) training data (y values)");
  params.addParam<unsigned int>("num_iters", 1000, "Tolerance value for Adam optimization");
  params.addParam<unsigned int>("batch_size", 0, "The batch size for Adam optimization");
  params.addParam<Real>("learning_rate", 0.001, "The learning rate for Adam optimization");
  params.addParam<unsigned int>(
      "show_every_nth_iteration",
      0,
      "Switch to show Adam optimization loss values at every nth step. If 0, nothing is showed.");
  params.addParam<std::vector<std::string>>(
      "tune_parameters", {}, "Select hyperparameters to be tuned");
  params.addParam<std::vector<Real>>("tuning_min", {}, "Minimum allowable tuning value");
  params.addParam<std::vector<Real>>("tuning_max", {}, "Maximum allowable tuning value");
  return params;
}

ActiveLearningGaussianProcess::ActiveLearningGaussianProcess(const InputParameters & parameters)
  : SurrogateTrainerBase(parameters),
    CovarianceInterface(parameters),
    SurrogateModelInterface(this),
    _gp(declareModelData<StochasticTools::GaussianProcess>("_gp")),
    _training_params(declareModelData<RealEigenMatrix>("_training_params")),
    _standardize_params(getParam<bool>("standardize_params")),
    _standardize_data(getParam<bool>("standardize_data")),
    _optimization_opts(StochasticTools::GaussianProcess::GPOptimizerOptions(
        getParam<unsigned int>("show_every_nth_iteration"),
        getParam<unsigned int>("num_iters"),
        getParam<unsigned int>("batch_size"),
        getParam<Real>("learning_rate")))
{
  _gp.initialize(getCovarianceFunctionByName(getParam<UserObjectName>("covariance_function")),
                 getParam<std::vector<std::string>>("tune_parameters"),
                 getParam<std::vector<Real>>("tuning_min"),
                 getParam<std::vector<Real>>("tuning_max"));
}

void
ActiveLearningGaussianProcess::reTrain(const std::vector<std::vector<Real>> & inputs,
                                       const std::vector<Real> & outputs) const
{

  // Addtional error check for each re-train call of the GP surrogate
  if (inputs.size() != outputs.size())
    mooseError("Number of inputs (",
               inputs.size(),
               ") does not match number of outputs (",
               outputs.size(),
               ").");
  if (inputs.empty())
    mooseError("There is no data for retraining.");

  RealEigenMatrix training_data;
  _training_params.setZero(outputs.size(), inputs[0].size());
  training_data.setZero(outputs.size(), 1);

  for (unsigned int i = 0; i < outputs.size(); ++i)
  {
    training_data(i, 0) = outputs[i];
    for (unsigned int j = 0; j < inputs[i].size(); ++j)
      _training_params(i, j) = inputs[i][j];
  }

  // Standardize (center and scale) training params
  if (_standardize_params)
    _gp.standardizeParameters(_training_params);
  // if not standardizing data set mean=0, std=1 for use in surrogate
  else
    _gp.paramStandardizer().set(0, 1, inputs[0].size());

  // Standardize (center and scale) training data
  if (_standardize_data)
    _gp.standardizeData(training_data);
  // if not standardizing data set mean=0, std=1 for use in surrogate
  else
    _gp.dataStandardizer().set(0, 1, inputs[0].size());

  // Setup the covariance
  _gp.setupCovarianceMatrix(_training_params, training_data, _optimization_opts);
}
