//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GaussianProcessTrainerTorched.h"
#include "Sampler.h"
#include "CartesianProduct.h"

#include <petsctao.h>
#include <petscdmda.h>

#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"

#include <cmath>

registerMooseObject("StochasticToolsApp", GaussianProcessTrainerTorched);

InputParameters
GaussianProcessTrainerTorched::validParams()
{
  InputParameters params = SurrogateTrainer::validParams();
  params.addClassDescription("Provides data preperation and training for a single- or multi-output "
                             "Gaussian Process surrogate model.");

  params.addRequiredParam<UserObjectName>("covariance_function_torched",
                                          "Name of covariance function.");
  params.addParam<bool>(
      "standardize_params", true, "Standardize (center and scale) training parameters (x values)");
  params.addParam<bool>(
      "standardize_data", true, "Standardize (center and scale) training data (y values)");
  // Already preparing to use Adam here
  params.addParam<unsigned int>("num_iters", 1000, "Tolerance value for Adam optimization");
  params.addParam<unsigned int>("batch_size", 0, "The batch size for Adam optimization");
  params.addParam<Real>("learning_rate", 0.001, "The learning rate for Adam optimization");
  params.addParam<unsigned int>(
      "show_every_nth_iteration",
      0,
      "Switch to show Adam optimization loss values at every nth step. If 0, nothing is showed.");
  params.addParam<std::vector<std::string>>("tune_parameters",
                                            "Select hyperparameters to be tuned");
  params.addParam<std::vector<Real>>("tuning_min", "Minimum allowable tuning value");
  params.addParam<std::vector<Real>>("tuning_max", "Maximum allowable tuning value");
  return params;
}

GaussianProcessTrainerTorched::GaussianProcessTrainerTorched(const InputParameters & parameters)
  : SurrogateTrainer(parameters),
    CovarianceInterfaceTorched(parameters),
    _predictor_row(getPredictorData()),
    _gp(declareModelData<StochasticToolsTorched::GaussianProcessTorched>("_gp")),
    _training_params(declareModelData<RealEigenMatrix>("_training_params")),
    _standardize_params(getParam<bool>("standardize_params")),
    _standardize_data(getParam<bool>("standardize_data")),
    _do_tuning(isParamValid("tune_parameters")),
    _optimization_opts(StochasticToolsTorched::GaussianProcessTorched::GPOptimizerOptions(
        getParam<unsigned int>("show_every_nth_iteration"),
        getParam<unsigned int>("num_iters"),
        getParam<unsigned int>("batch_size"),
        getParam<Real>("learning_rate"))),
    _sampler_row(getSamplerData())
{
  // Error Checking
  if (parameters.isParamSetByUser("batch_size"))
    if (_sampler.getNumberOfRows() < _optimization_opts.batch_size)
      paramError("batch_size", "Batch size cannot be greater than the training data set size.");

  std::vector<std::string> tune_parameters(
      _do_tuning ? getParam<std::vector<std::string>>("tune_parameters")
                 : std::vector<std::string>{});

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

  _gp.initialize(
      getCovarianceFunctionByName(parameters.get<UserObjectName>("covariance_function_torched")),
      tune_parameters,
      lower_bounds,
      upper_bounds);

  _n_outputs = _gp.getCovarFunction().numOutputs();
}

void
GaussianProcessTrainerTorched::preTrain()
{
  _params_buffer.clear();
  _data_buffer.clear();
  _params_buffer.reserve(getLocalSampleSize());
  _data_buffer.reserve(getLocalSampleSize());
}

void
GaussianProcessTrainerTorched::train()
{
  _params_buffer.push_back(_predictor_row);

  if (_rvecval && _rvecval->size() != _n_outputs)
    mooseError("The size of the provided response (",
               _rvecval->size(),
               ") does not match the number of expected outputs from the covariance (",
               _n_outputs,
               ")!");

  _data_buffer.push_back(_rvecval ? (*_rvecval) : std::vector<Real>(1, *_rval));
}

void
GaussianProcessTrainerTorched::postTrain()
{
  // Instead of gatherSum, we have to allgather.
  _communicator.allgather(_params_buffer);
  _communicator.allgather(_data_buffer);

  _training_params.resize(_params_buffer.size(), _n_dims);
  _training_data.resize(_data_buffer.size(), _n_outputs);

  for (auto ii : make_range(_training_params.rows()))
  {
    for (auto jj : make_range(_n_dims))
      _training_params(ii, jj) = _params_buffer[ii][jj];
    for (auto jj : make_range(_n_outputs))
      _training_data(ii, jj) = _data_buffer[ii][jj];
  }

  auto options = torch::TensorOptions().dtype(at::kDouble);
  torch::Tensor training_tensor =
      torch::from_blob(
          _training_params.data(), {_training_params.cols(), _training_params.rows()}, options)
          .clone();
  torch::Tensor training_tensor_transposed = training_tensor.transpose(1, 0);

  torch::Tensor training_data_tensor =
      torch::from_blob(
          _training_data.data(), {_training_data.rows(), _training_data.cols()}, options)
          .to(at::kDouble);

  // Standardize (center and scale) training params

  // TODO: After standardizing param and data tensor, update values of Eigen::matrix versions to
  // match
  if (_standardize_params)
    _gp.standardizeParameters(training_tensor_transposed);
  // if not standardizing data set mean=0, std=1 for use in surrogate
  else
    _gp.paramStandardizer().set(0, 1, _n_dims);

  // Standardize (center and scale) training data
  if (_standardize_data)
    _gp.standardizeData(training_data_tensor);
  // if not standardizing data set mean=0, std=1 for use in surrogate
  else
    _gp.dataStandardizer().set(0, 1, _n_outputs);

  // Setup the covariance
  _gp.setupCovarianceMatrix(training_tensor_transposed, training_data_tensor, _optimization_opts);

  // Update _training_params with values from tensor. Once _training_params is replaced with a
  // torch::Tensor remove this
  auto training_accessor = training_tensor_transposed.accessor<Real, 2>();
  for (unsigned int ii = 0; ii < training_tensor_transposed.sizes()[0]; ii++)
  {
    for (unsigned int jj = 0; jj < training_tensor_transposed.sizes()[1]; jj++)
    {
      _training_params(ii, jj) = training_accessor[ii][jj];
    }
  }
}
