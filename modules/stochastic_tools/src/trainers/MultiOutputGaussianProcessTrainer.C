//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiOutputGaussianProcessTrainer.h"
#include "Sampler.h"
#include "CartesianProduct.h"

#include <petsctao.h>
#include <petscdmda.h>

#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"

#include <cmath>

registerMooseObject("StochasticToolsApp", MultiOutputGaussianProcessTrainer);

InputParameters
MultiOutputGaussianProcessTrainer::validParams()
{
  InputParameters params = SurrogateTrainer::validParams();
  params.addClassDescription("Provides data preperation and training for a Multi-Output Gaussian "
                             "Process surrogate model.");
  params.addRequiredParam<UserObjectName>("output_covariance", "Name of output covariance.");
  params.addRequiredParam<UserObjectName>("covariance_function", "Name of covariance function.");
  params.addParam<unsigned int>("iterations", 1000, "Tolerance value for Adam optimization");
  params.addParam<unsigned int>("batch_size", 0, "The batch size for Adam optimization");
  params.addParam<Real>("learning_rate", 0.001, "The learning rate for Adam optimization");
  params.addParam<bool>("show_optimization_details", false, "Switch to show Adam solver results");
  params.addParam<std::vector<std::string>>("tune_parameters",
                                            "Select hyperparameters to be tuned");
  return params;
}

MultiOutputGaussianProcessTrainer::MultiOutputGaussianProcessTrainer(
    const InputParameters & parameters)
  : SurrogateTrainer(parameters),
    CovarianceInterface(parameters),
    OutputCovarianceInterface(parameters),
    _predictor_row(getPredictorData()),
    _mogp_handler(
        declareModelData<StochasticTools::MultiOutputGaussianProcessHandler>("_mogp_handler")),
    _training_params(declareModelData<RealEigenMatrix>("_training_params")),
    _optimization_opts(StochasticTools::MultiOutputGaussianProcessHandler::GPOptimizerOptions(
        getParam<bool>("show_optimization_details"),
        getParam<unsigned int>("iterations"),
        getParam<unsigned int>("batch_size"),
        getParam<Real>("learning_rate"))),
    _sampler_row(getSamplerData()),
    _pvals(getParam<std::vector<ReporterName>>("predictors").size()),
    _pcols(getParam<std::vector<unsigned int>>("predictor_cols")),
    _n_params((_pvals.empty() && _pcols.empty()) ? _sampler.getNumberOfCols()
                                                 : (_pvals.size() + _pcols.size()))
{
  if (parameters.isParamSetByUser("batch_size"))
    if (_sampler.getNumberOfRows() < _optimization_opts.batch_size)
      paramError("batch_size", "Batch size cannot be greater than the training data set size.");

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
  std::vector<Real> lower_bounds, upper_bounds;
  _mogp_handler.initialize(
      getOutputCovarianceByName(parameters.get<UserObjectName>("output_covariance")),
      getCovarianceFunctionByName(parameters.get<UserObjectName>("covariance_function")),
      tune_parameters,
      lower_bounds,
      upper_bounds);
}

void
MultiOutputGaussianProcessTrainer::preTrain()
{
  _params_buffer.clear();
  _data_buffer.clear();
  _params_buffer.reserve(getLocalSampleSize());
  _data_buffer.reserve(getLocalSampleSize());
}

void
MultiOutputGaussianProcessTrainer::train()
{
  _params_buffer.push_back(_predictor_row);
  _data_buffer.push_back(*_rvecval);
}

void
MultiOutputGaussianProcessTrainer::postTrain()
{
  // Instead of gatherSum, we have to allgather.
  _communicator.allgather(_params_buffer);
  _communicator.allgather(_data_buffer);

  // Set up the params and data matrices
  _training_params.resize(_params_buffer.size(), _n_dims);
  _training_data.resize(_params_buffer.size(), _data_buffer[0].size());
  for (auto ii : make_range(_training_params.rows()))
  {
    for (auto jj : make_range(_training_params.cols()))
      _training_params(ii, jj) = _params_buffer[ii][jj];
    for (auto jj : make_range(_training_data.cols()))
      _training_data(ii, jj) = _data_buffer[ii][jj];
  }

  // Standardize (center and scale) training params
  _mogp_handler.standardizeParameters(_training_params);

  // Standardize (center and scale) training data
  _mogp_handler.standardizeData(_training_data);

  // Setup the covariance
  _mogp_handler.setupCovarianceMatrix(_training_params, _training_data, _optimization_opts);
}
