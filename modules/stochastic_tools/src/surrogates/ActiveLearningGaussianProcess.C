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
  MooseEnum tuning_type("tao adam none", "none");
  params.addParam<MooseEnum>(
      "tuning_algorithm", tuning_type, "Hyper parameter optimizaton algorithm");
  params.addParam<unsigned int>("iter_adam", 1000, "Tolerance value for Adam optimization");
  params.addParam<unsigned int>("batch_size", 0, "The batch size for Adam optimization");
  params.addParam<Real>("learning_rate_adam", 0.001, "The learning rate for Adam optimization");
  params.addParam<std::string>(
      "tao_options", "", "Command line options for PETSc/TAO hyperparameter optimization");
  params.addParam<bool>(
      "show_optimization_details", false, "Switch to show TAO or Adam solver results");
  params.addParam<std::vector<std::string>>("tune_parameters",
                                            "Select hyperparameters to be tuned");
  params.addParam<std::vector<Real>>("tuning_min", "Minimum allowable tuning value");
  params.addParam<std::vector<Real>>("tuning_max", "Maximum allowable tuning value");
  return params;
}

ActiveLearningGaussianProcess::ActiveLearningGaussianProcess(const InputParameters & parameters)
  : SurrogateTrainerBase(parameters),
    CovarianceInterface(parameters),
    SurrogateModelInterface(this),
    _gp_handler(declareModelData<StochasticTools::GaussianProcessHandler>("_gp_handler")),
    _training_params(declareModelData<RealEigenMatrix>("_training_params")),
    _standardize_params(getParam<bool>("standardize_params")),
    _standardize_data(getParam<bool>("standardize_data")),
    _optimization_opts(StochasticTools::GaussianProcessHandler::GPOptimizerOptions(
        getParam<MooseEnum>("tuning_algorithm"),
        getParam<std::string>("tao_options"),
        getParam<bool>("show_optimization_details"),
        getParam<unsigned int>("iter_adam"),
        getParam<unsigned int>("batch_size"),
        getParam<Real>("learning_rate_adam")))
{

// Error Checking
//   if (_do_tuning && _tuning_algorithm == "none")
//     paramError("tuning_algorithm",
//                "No tuning algorithm is selected for the hyper parameter optimization!");

//   if (isParamValid("batch_size") && _tuning_algorithm == "tao")
//     mooseError("Mini-batch sampling is not compatible with the TAO optimization library. Please "
//                "use Adam optimization.");

//   if (!isParamValid("batch_size") && _tuning_algorithm == "adam")
//     paramError("batch_size", "Adam requires the batch size to be specified.");

//   if (isParamValid("batch_size"))
//     if (_sampler.getNumberOfRows() < _batch_size)
//       paramError("batch_size", "Batch size cannot be greater than the training data set size.");

  std::vector<std::string> tune_parameters(getParam<std::vector<std::string>>("tune_parameters"));

  std::vector<Real> bounds;
  _gp_handler.initialize(
      getCovarianceFunctionByName(parameters.get<UserObjectName>("covariance_function")),
      tune_parameters,
      bounds,
      bounds);
}

void 
ActiveLearningGaussianProcess::reTrain(const std::vector<std::vector<Real>> & inputs, const std::vector<Real> & outputs) const
{

  // RealEigenMatrix _training_params;
  RealEigenMatrix _training_data;

  _training_params.setZero(outputs.size(), inputs.size());
  _training_data.setZero(outputs.size(), 1);

  for (unsigned int i = 0; i < outputs.size(); ++i)
  {
    _training_data(i, 0) = outputs[i];
    for (unsigned int j = 0; j < inputs.size(); ++j)
      _training_params(i,j) = inputs[j][i];
  }

  // Standardize (center and scale) training params
  if (_standardize_params)
    _gp_handler.standardizeParameters(_training_params);
  // if not standardizing data set mean=0, std=1 for use in surrogate
  else
    _gp_handler.paramStandardizer().set(0, 1, inputs.size());

  // Standardize (center and scale) training data
  if (_standardize_data)
    _gp_handler.standardizeData(_training_data);
  // if not standardizing data set mean=0, std=1 for use in surrogate
  else
    _gp_handler.dataStandardizer().set(0, 1, inputs.size());

  // Setup the covariance
  _gp_handler.setupCovarianceMatrix(_training_params, _training_data, _optimization_opts);
}