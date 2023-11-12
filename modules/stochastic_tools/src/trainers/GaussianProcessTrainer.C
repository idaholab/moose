//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GaussianProcessTrainer.h"
#include "Sampler.h"
#include "CartesianProduct.h"

#include <petsctao.h>
#include <petscdmda.h>

#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"

#include <cmath>

registerMooseObject("StochasticToolsApp", GaussianProcessTrainer);

InputParameters
GaussianProcessTrainer::validParams()
{
  InputParameters params = SurrogateTrainer::validParams();
  params.addClassDescription(
      "Provides data preperation and training for a Gaussian Process surrogate model.");

  params.addRequiredParam<UserObjectName>("covariance_function", "Name of covariance function.");
  params.addParam<bool>(
      "standardize_params", true, "Standardize (center and scale) training parameters (x values)");
  params.addParam<bool>(
      "standardize_data", true, "Standardize (center and scale) training data (y values)");
  // Already preparing to use Adam here
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

  params.suppressParameter<MooseEnum>("response_type");
  return params;
}

GaussianProcessTrainer::GaussianProcessTrainer(const InputParameters & parameters)
  : SurrogateTrainer(parameters),
    CovarianceInterface(parameters),
    _predictor_row(getPredictorData()),
    _gp_handler(declareModelData<StochasticTools::GaussianProcessHandler>("_gp_handler")),
    _training_params(declareModelData<RealEigenMatrix>("_training_params")),
    _standardize_params(getParam<bool>("standardize_params")),
    _standardize_data(getParam<bool>("standardize_data")),
    _do_tuning(isParamValid("tune_parameters")),
    _optimization_opts(StochasticTools::GaussianProcessHandler::GPOptimizerOptions(
        getParam<MooseEnum>("tuning_algorithm"),
        getParam<std::string>("tao_options"),
        getParam<bool>("show_optimization_details"),
        getParam<unsigned int>("iter_adam"),
        getParam<unsigned int>("batch_size"),
        getParam<Real>("learning_rate_adam"))),
    _sampler_row(getSamplerData()),
    _pvals(getParam<std::vector<ReporterName>>("predictors").size()),
    _pcols(getParam<std::vector<unsigned int>>("predictor_cols")),
    _n_params((_pvals.empty() && _pcols.empty()) ? _sampler.getNumberOfCols()
                                                 : (_pvals.size() + _pcols.size()))
{
  const auto & pnames = getParam<std::vector<ReporterName>>("predictors");
  for (unsigned int i = 0; i < pnames.size(); ++i)
    _pvals[i] = &getTrainingData<Real>(pnames[i]);

  // If predictors and predictor_cols are empty, use all sampler columns
  if (_pvals.empty() && _pcols.empty())
  {
    _pcols.resize(_sampler.getNumberOfCols());
    std::iota(_pcols.begin(), _pcols.end(), 0);
  }

  // Error Checking
  if (_do_tuning && _optimization_opts.opt_type == "none")
    paramError("tuning_algorithm",
               "No tuning algorithm is selected for the hyper parameter optimization!");

  if (parameters.isParamSetByUser("batch_size") && _optimization_opts.opt_type == "tao")
    paramError("batch_size",
               "Mini-batch sampling is not compatible with the TAO optimization library. Please "
               "use Adam optimization.");

  if (parameters.isParamSetByUser("batch_size"))
    if (_sampler.getNumberOfRows() < _optimization_opts.batch_size)
      paramError("batch_size", "Batch size cannot be greater than the training data set size.");

  std::vector<std::string> tune_parameters(getParam<std::vector<std::string>>("tune_parameters"));

  if (isParamValid("tuning_min") &&
      (getParam<std::vector<Real>>("tuning_min").size() != tune_parameters.size()))
    mooseError("tuning_min size does not match tune_parameters");
  if (isParamValid("tuning_max") &&
      (getParam<std::vector<Real>>("tuning_max").size() != tune_parameters.size()))
    mooseError("tuning_max size does not match tune_parameters");

  std::vector<Real> lower_bounds, upper_bounds;
  if (isParamValid("tuning_min"))
    lower_bounds = getParam<std::vector<Real>>("tuning_min");
  if (isParamValid("tuning_max"))
    upper_bounds = getParam<std::vector<Real>>("tuning_max");

  _gp_handler.initialize(
      getCovarianceFunctionByName(parameters.get<UserObjectName>("covariance_function")),
      tune_parameters,
      lower_bounds,
      upper_bounds);
}

void
GaussianProcessTrainer::preTrain()
{
  _params_buffer.clear();
  _data_buffer.clear();
  _params_buffer.reserve(getLocalSampleSize());
  _data_buffer.reserve(getLocalSampleSize());
}

void
GaussianProcessTrainer::train()
{
  _params_buffer.push_back(_predictor_row);
  _data_buffer.push_back(*_rval);
}

void
GaussianProcessTrainer::postTrain()
{
  // Instead of gatherSum, we have to allgather.
  _communicator.allgather(_params_buffer);
  _communicator.allgather(_data_buffer);

  _training_params.resize(_params_buffer.size(), _n_dims);
  _training_data.resize(_data_buffer.size(), 1);

  for (auto ii : make_range(_training_params.rows()))
  {
    for (auto jj : make_range(_training_params.cols()))
      _training_params(ii, jj) = _params_buffer[ii][jj];
    _training_data(ii, 0) = _data_buffer[ii];
  }

  // Standardize (center and scale) training params
  if (_standardize_params)
    _gp_handler.standardizeParameters(_training_params);
  // if not standardizing data set mean=0, std=1 for use in surrogate
  else
    _gp_handler.paramStandardizer().set(0, 1, _n_dims);

  // Standardize (center and scale) training data
  if (_standardize_data)
    _gp_handler.standardizeData(_training_data);
  // if not standardizing data set mean=0, std=1 for use in surrogate
  else
    _gp_handler.dataStandardizer().set(0, 1, _n_dims);

  // Setup the covariance
  _gp_handler.setupCovarianceMatrix(_training_params, _training_data, _optimization_opts);
}
