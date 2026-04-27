//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#include <limits>

registerMooseObject("StochasticToolsApp", GaussianProcessTrainer);

InputParameters
GaussianProcessTrainer::validParams()
{
  InputParameters params = SurrogateTrainer::validParams();
  params.addClassDescription("Provides data preperation and training for a single- or multi-output "
                             "Gaussian Process surrogate model.");

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
  params.addParam<std::vector<std::string>>("tune_parameters",
                                            "Select hyperparameters to be tuned");
  params.addParam<std::vector<Real>>("tuning_min", "Minimum allowable tuning value");
  params.addParam<std::vector<Real>>("tuning_max", "Maximum allowable tuning value");

  // ---- Link function parameters ----
  MooseEnum link_function_options("identity log logit", "identity");
  params.addParam<MooseEnum>(
      "link_function",
      link_function_options,
      "Output link function enforcing inequality constraints. 'identity' (default): no constraint; "
      "'log': enforces y > link_lower_bound (e.g. positivity); "
      "'logit': enforces link_lower_bound < y < link_upper_bound.");
  params.addParam<Real>(
      "link_lower_bound", 0.0, "Lower bound for the 'log' or 'logit' link function.");
  params.addParam<Real>("link_upper_bound", 1.0, "Upper bound for the 'logit' link function.");

  // ---- Derivative observation (monotonicity constraint) parameters ----
  params.addParam<std::vector<Real>>(
      "monotone_constraint_points",
      "Locations of virtual derivative observations as a flat row-major vector "
      "(n_points x n_dims). Used to enforce monotonicity or zero-derivative constraints. "
      "Requires a covariance kernel that supports derivative covariances (e.g. "
      "SquaredExponentialCovariance).");
  params.addParam<std::vector<unsigned int>>(
      "monotone_constraint_dims",
      "Input dimension index for each virtual derivative observation. Must have the same "
      "number of entries as the number of points in monotone_constraint_points.");
  MooseEnum constraint_type_options("zero increasing decreasing", "zero");
  params.addParam<MooseEnum>(
      "monotone_constraint_type",
      constraint_type_options,
      "Target derivative type: 'zero' (df/dx_k ≈ 0), 'increasing' (df/dx_k > 0), "
      "or 'decreasing' (df/dx_k < 0).");
  params.addParam<Real>("derivative_target_value",
                        0.0,
                        "Magnitude of the target derivative for 'increasing' or 'decreasing' "
                        "constraints, in the standardized output space. Default 0 means the "
                        "constraint softly pins the derivative to zero.");
  params.addParam<Real>(
      "derivative_noise_variance",
      1e-4,
      "Noise variance (sigma_d^2) added to the derivative-derivative diagonal blocks of the "
      "augmented covariance matrix. Must be > 0. Smaller values enforce constraints more strictly "
      "but may cause numerical issues.");

  // ---- Penalty constraint parameters ----
  params.addParam<std::vector<Real>>(
      "penalty_constraint_points",
      "Locations of penalty constraint evaluation points as a flat row-major vector "
      "(n_points x n_dims). The penalty is added to the NLML loss and gradient during "
      "hyperparameter tuning when predicted means violate the specified bounds.");
  params.addParam<std::vector<Real>>(
      "penalty_constraint_lower_bounds",
      "Lower bounds on the GP output (in physical space, before link function) at each "
      "penalty constraint point. Use -1e30 or similar to disable a lower bound.");
  params.addParam<std::vector<Real>>(
      "penalty_constraint_upper_bounds",
      "Upper bounds on the GP output (in physical space, before link function) at each "
      "penalty constraint point. Use 1e30 or similar to disable an upper bound.");
  params.addParam<Real>(
      "penalty_weight",
      1.0,
      "Weight lambda for the penalty constraint term: lambda * sum_c max(0, violation_c)^2.");

  return params;
}

GaussianProcessTrainer::GaussianProcessTrainer(const InputParameters & parameters)
  : SurrogateTrainer(parameters),
    CovarianceInterface(parameters),
    _predictor_row(getPredictorData()),
    _gp(declareModelData<StochasticTools::GaussianProcess>("_gp")),
    _training_params(declareModelData<RealEigenMatrix>("_training_params")),
    _standardize_params(getParam<bool>("standardize_params")),
    _standardize_data(getParam<bool>("standardize_data")),
    _do_tuning(isParamValid("tune_parameters")),
    _optimization_opts(StochasticTools::GaussianProcess::GPOptimizerOptions(
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

  _gp.initialize(getCovarianceFunctionByName(parameters.get<UserObjectName>("covariance_function")),
                 tune_parameters,
                 lower_bounds,
                 upper_bounds);

  _n_outputs = _gp.getCovarFunction().numOutputs();
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

  if (_rvecval && _rvecval->size() != _n_outputs)
    mooseError("The size of the provided response (",
               _rvecval->size(),
               ") does not match the number of expected outputs from the covariance (",
               _n_outputs,
               ")!");

  _data_buffer.push_back(_rvecval ? (*_rvecval) : std::vector<Real>(1, *_rval));
}

void
GaussianProcessTrainer::postTrain()
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

  // ---- Link function ----
  const std::string link_str = getParam<MooseEnum>("link_function");
  StochasticTools::GPLinkFunctionType link_type = StochasticTools::GPLinkFunctionType::Identity;
  if (link_str == "log")
    link_type = StochasticTools::GPLinkFunctionType::Log;
  else if (link_str == "logit")
    link_type = StochasticTools::GPLinkFunctionType::Logit;
  const Real link_lb = getParam<Real>("link_lower_bound");
  const Real link_ub = getParam<Real>("link_upper_bound");
  if (link_str == "logit" && link_ub <= link_lb)
    paramError("link_upper_bound",
               "link_upper_bound must be strictly greater than link_lower_bound for logit link.");
  _gp.setLinkFunction(link_type, link_lb, link_ub);

  // Apply link function to training data BEFORE standardization
  _gp.applyLinkTransform(_training_data);

  // Standardize (center and scale) training params
  if (_standardize_params)
    _gp.standardizeParameters(_training_params);
  else
    _gp.paramStandardizer().set(0, 1, _n_dims);

  // Standardize (center and scale) training data (in link-transformed z-space)
  if (_standardize_data)
    _gp.standardizeData(_training_data);
  else
    _gp.dataStandardizer().set(0, 1, _n_outputs);

  // ---- Derivative constraint (virtual observations) ----
  if (isParamValid("monotone_constraint_points"))
  {
    const auto & raw_pts = getParam<std::vector<Real>>("monotone_constraint_points");
    if (!isParamValid("monotone_constraint_dims"))
      paramError("monotone_constraint_dims",
                 "monotone_constraint_dims is required when monotone_constraint_points is set.");
    const auto & dims = getParam<std::vector<unsigned int>>("monotone_constraint_dims");
    const unsigned int n_virt = dims.size();
    if (raw_pts.size() != n_virt * (unsigned)_n_dims)
      paramError("monotone_constraint_points",
                 "monotone_constraint_points must contain n_points * n_dims = ",
                 n_virt,
                 " * ",
                 _n_dims,
                 " = ",
                 n_virt * _n_dims,
                 " values. Got ",
                 raw_pts.size(),
                 ".");

    // Fill virtual_params in physical space, then standardize
    RealEigenMatrix virtual_params(n_virt, _n_dims);
    for (unsigned int i = 0; i < n_virt; ++i)
      for (unsigned int j = 0; j < (unsigned)_n_dims; ++j)
        virtual_params(i, j) = raw_pts[i * _n_dims + j];

    _gp.getParamStandardizer().getStandardized(virtual_params);

    const std::string ctype = getParam<MooseEnum>("monotone_constraint_type");
    const Real magnitude = getParam<Real>("derivative_target_value");
    Real target_value = 0.0;
    if (ctype == "increasing")
      target_value = magnitude;
    else if (ctype == "decreasing")
      target_value = -magnitude;

    _gp.setDerivativeConstraints(
        virtual_params, dims, target_value, getParam<Real>("derivative_noise_variance"));
  }

  // ---- Penalty constraints ----
  if (isParamValid("penalty_constraint_points"))
  {
    const auto & raw_pts = getParam<std::vector<Real>>("penalty_constraint_points");
    const auto & lower_raw = isParamValid("penalty_constraint_lower_bounds")
                                 ? getParam<std::vector<Real>>("penalty_constraint_lower_bounds")
                                 : std::vector<Real>{};
    const auto & upper_raw = isParamValid("penalty_constraint_upper_bounds")
                                 ? getParam<std::vector<Real>>("penalty_constraint_upper_bounds")
                                 : std::vector<Real>{};

    const unsigned int n_penalty = lower_raw.empty() ? upper_raw.size() : lower_raw.size();
    if (!lower_raw.empty() && !upper_raw.empty() && lower_raw.size() != upper_raw.size())
      paramError("penalty_constraint_lower_bounds",
                 "penalty_constraint_lower_bounds and penalty_constraint_upper_bounds must have "
                 "the same number of entries.");
    if (raw_pts.size() != n_penalty * (unsigned)_n_dims)
      paramError("penalty_constraint_points",
                 "penalty_constraint_points must have n_penalty * n_dims entries.");

    RealEigenMatrix penalty_pts(n_penalty, _n_dims);
    for (unsigned int i = 0; i < n_penalty; ++i)
      for (unsigned int j = 0; j < (unsigned)_n_dims; ++j)
        penalty_pts(i, j) = raw_pts[i * _n_dims + j];

    _gp.getParamStandardizer().getStandardized(penalty_pts);

    // Transform bounds through link function then standardize into z-space
    const Real neg_inf = -std::numeric_limits<Real>::max();
    const Real pos_inf = std::numeric_limits<Real>::max();
    std::vector<Real> lower_std(n_penalty, neg_inf), upper_std(n_penalty, pos_inf);
    const Real z_mean = _gp.getDataStandardizer().getMean()[0];
    const Real z_std_val = _gp.getDataStandardizer().getStdDev()[0];
    for (unsigned int c = 0; c < n_penalty; ++c)
    {
      if (!lower_raw.empty() && lower_raw[c] > neg_inf)
      {
        const Real z_lb = _gp.applyLink(lower_raw[c]);
        lower_std[c] = (z_lb - z_mean) / z_std_val;
      }
      if (!upper_raw.empty() && upper_raw[c] < pos_inf)
      {
        const Real z_ub = _gp.applyLink(upper_raw[c]);
        upper_std[c] = (z_ub - z_mean) / z_std_val;
      }
    }

    _gp.setPenaltyConstraints(penalty_pts, lower_std, upper_std, getParam<Real>("penalty_weight"));
  }

  // Setup the covariance
  _gp.setupCovarianceMatrix(_training_params, _training_data, _optimization_opts);
}
