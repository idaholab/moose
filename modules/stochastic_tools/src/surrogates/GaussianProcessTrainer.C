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

#include <math.h>

registerMooseObject("StochasticToolsApp", GaussianProcessTrainer);

InputParameters
GaussianProcessTrainer::validParams()
{
  InputParameters params = SurrogateTrainer::validParams();
  params += CovarianceInterface::validParams();
  params.addClassDescription(
      "Provides data preperation and training for a Gaussian Process surrogate model.");
  params.addRequiredParam<ReporterName>(
      "response", "Reporter value of response results, can be vpp with <vpp_name>/<vector_name>.");
  params.addParam<std::vector<ReporterName>>(
      "predictors",
      std::vector<ReporterName>(),
      "Reporter values used as the independent random variables, If 'predictors' and "
      "'predictor_cols' are both empty, all sampler columns are used.");
  params.addParam<std::vector<unsigned int>>(
      "predictor_cols",
      std::vector<unsigned int>(),
      "Sampler columns used as the independent random variables, If 'predictors' and "
      "'predictor_cols' are both empty, all sampler columns are used.");
  params.addRequiredParam<UserObjectName>("covariance_function", "Name of covariance function.");
  params.addParam<bool>(
      "standardize_params", true, "Standardize (center and scale) training parameters (x values)");
  params.addParam<bool>(
      "standardize_data", true, "Standardize (center and scale) training data (y values)");
  params.addParam<std::string>(
      "tao_options", "", "Command line options for PETSc/TAO hyperparameter optimization");
  params.addParam<bool>("show_tao", false, "Switch to show TAO solver results");
  params.addParam<std::vector<std::string>>("tune_parameters",
                                            "Select hyperparameters to be tuned");
  params.addParam<std::vector<Real>>("tuning_min", "Minimum allowable tuning value");
  params.addParam<std::vector<Real>>("tuning_max", "Maximum allowable tuning value");
  return params;
}

GaussianProcessTrainer::GaussianProcessTrainer(const InputParameters & parameters)
  : SurrogateTrainer(parameters),
    CovarianceInterface(parameters),
    _gp_utils(declareModelData<StochasticTools::GaussianProcessUtils>("_gp_utils")),
    _training_params(declareModelData<RealEigenMatrix>("_training_params")),
    _standardize_params(getParam<bool>("standardize_params")),
    _standardize_data(getParam<bool>("standardize_data")),
    _do_tuning(isParamValid("tune_parameters")),
    _tao_options(getParam<std::string>("tao_options")),
    _show_tao(getParam<bool>("show_tao")),
    _tao_comm(MPI_COMM_SELF),
    _sampler_row(getSamplerData()),
    _rval(getTrainingData<Real>(getParam<ReporterName>("response"))),
    _pvals(getParam<std::vector<ReporterName>>("predictors").size()),
    _pcols(getParam<std::vector<unsigned int>>("predictor_cols")),
    _n_params((_pvals.empty() && _pcols.empty()) ? _sampler.getNumberOfCols()
                                                 : (_pvals.size() + _pcols.size()))
{
  _gp_utils.linkCovarianceFunction(parameters,
                                   parameters.get<UserObjectName>("covariance_function"));
  const auto & pnames = getParam<std::vector<ReporterName>>("predictors");
  for (unsigned int i = 0; i < pnames.size(); ++i)
    _pvals[i] = &getTrainingData<Real>(pnames[i]);

  // If predictors and predictor_cols are empty, use all sampler columns
  if (_pvals.empty() && _pcols.empty())
  {
    _pcols.resize(_sampler.getNumberOfCols());
    std::iota(_pcols.begin(), _pcols.end(), 0);
  }

  std::vector<std::string> tune_parameters(getParam<std::vector<std::string>>("tune_parameters"));
  // Error Checking
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

  _gp_utils.generateTuningMap(tune_parameters, lower_bounds, upper_bounds);
}

void
GaussianProcessTrainer::preTrain()
{
  _training_params.setZero(_sampler.getNumberOfRows(), _n_params);
  _training_data.setZero(_sampler.getNumberOfRows(), 1);
}

void
GaussianProcessTrainer::train()
{
  unsigned int d = 0;
  for (const auto & val : _pvals)
    _training_params(_row, d++) = *val;
  for (const auto & col : _pcols)
    _training_params(_row, d++) = _sampler_row[col];

  // Loading result data from response reporter
  _training_data(_row, 0) = _rval;
}

void
GaussianProcessTrainer::postTrain()
{
  for (unsigned int ii = 0; ii < _training_params.rows(); ++ii)
  {
    for (unsigned int jj = 0; jj < _training_params.cols(); ++jj)
      gatherSum(_training_params(ii, jj));
    gatherSum(_training_data(ii, 0));
  }

  // Standardize (center and scale) training params
  if (_standardize_params)
    _gp_utils.standardizeParameters(_training_params);
  // if not standardizing data set mean=0, std=1 for use in surrogate
  else
    _gp_utils.paramStandardizer().set(0, 1, _n_params);

  // Standardize (center and scale) training data
  if (_standardize_data)
    _gp_utils.standardizeData(_training_data);
  // if not standardizing data set mean=0, std=1 for use in surrogate
  else
    _gp_utils.dataStandardizer().set(0, 1, _n_params);

  _gp_utils.K().resize(_training_params.rows(), _training_params.rows());

  if (_do_tuning)
  {
    if (_gp_utils.tuneHyperParamsTAO(_training_params, _training_data, _tao_options, _show_tao))
      mooseError("PETSc/TAO error in hyperparameter tuning.");
  }

  _gp_utils.covarFunction().computeCovarianceMatrix(
      _gp_utils.K(), _training_params, _training_params, true);
  _gp_utils.setupStoredMatrices(_training_data);

  _gp_utils.covarFunction().buildHyperParamMap(_gp_utils.hyperparamMap(),
                                               _gp_utils.hyperparamVectorMap());
}
