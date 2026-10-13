//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#ifdef MOOSE_LIBTORCH_ENABLED

#include "GaussianProcess.h"
#include "FEProblemBase.h"

#include <petsctao.h>
#include <petscdmda.h>

#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"

#include <cmath>
#include <limits>

#include "MooseRandom.h"
#include "Shuffle.h"

#include <torch/optim/adam.h>

namespace StochasticTools
{

namespace
{

using HyperParameterMap = GaussianProcess::HyperParameterMap;

torch::Tensor
flattenOutputData(const torch::Tensor & output_data)
{
  mooseAssert(output_data.dim() == 2, "GaussianProcess output data must be rank-2.");
  return torch::reshape(torch::transpose(output_data, 0, 1),
                        {output_data.size(0) * output_data.size(1), 1});
}

torch::TensorOptions
doubleOptionsLike(const torch::Tensor & tensor)
{
  return tensor.options().dtype(at::kDouble);
}

torch::Tensor
toOptions(const torch::Tensor & tensor, const torch::TensorOptions & options)
{
  auto result = tensor.to(options.device());
  if (result.scalar_type() != at::kDouble)
    result = result.to(at::kDouble);
  return result;
}

std::vector<Real>
exportHyperParameter(const torch::Tensor & tensor)
{
  if (!CovarianceFunctionBase::isScalarHyperParameter(tensor) &&
      !CovarianceFunctionBase::isVectorHyperParameter(tensor))
    mooseError("Unsupported hyperparameter rank ", tensor.dim(), ".");
  auto cpu_tensor = LibtorchUtils::toCPUContiguous(tensor);
  if (cpu_tensor.scalar_type() != at::kDouble)
    cpu_tensor = cpu_tensor.to(at::kDouble).contiguous();
  const auto flattened = cpu_tensor.reshape({-1});
  return {flattened.data_ptr<Real>(), flattened.data_ptr<Real>() + flattened.numel()};
}

torch::Tensor
buildVectorHyperParameter(const std::vector<Real> & values, const torch::TensorOptions & options)
{
  auto tensor = torch::empty({long(values.size())}, torch::TensorOptions().dtype(at::kDouble));
  auto tensor_accessor = tensor.accessor<Real, 1>();
  for (const auto index : index_range(values))
    tensor_accessor[index] = values[index];
  return toOptions(tensor, options);
}

void
moveHyperParameters(HyperParameterMap & hyperparameters, const torch::TensorOptions & options)
{
  for (auto & iter : hyperparameters)
    iter.second = toOptions(iter.second, options);
}

void
updateHyperParameter(torch::Tensor & tensor,
                     const std::vector<Real> & values,
                     const std::string & name)
{
  const auto options = doubleOptionsLike(tensor);
  if (CovarianceFunctionBase::isScalarHyperParameter(tensor))
  {
    mooseAssert(values.size() == 1, "Scalar hyperparameter update requires a single value.");
    tensor =
        toOptions(torch::tensor(values[0], torch::TensorOptions().dtype(at::kDouble)), options);
  }
  else if (CovarianceFunctionBase::isVectorHyperParameter(tensor))
    tensor = buildVectorHyperParameter(values, options);
  else
    mooseError("Unsupported hyperparameter rank ", tensor.dim(), " for ", name, ".");
}

} // namespace

GaussianProcess::GPOptimizerOptions::GPOptimizerOptions(const unsigned int show_every_nth_iteration,
                                                        const unsigned int num_iter,
                                                        const unsigned int batch_size,
                                                        const Real learning_rate,
                                                        const Real b1,
                                                        const Real b2,
                                                        const Real eps,
                                                        const Real lambda,
                                                        const OptimizerType optimizer_type)
  : show_every_nth_iteration(show_every_nth_iteration),
    num_iter(num_iter),
    batch_size(batch_size),
    learning_rate(learning_rate),
    b1(b1),
    b2(b2),
    eps(eps),
    lambda(lambda),
    optimizer_type(optimizer_type)
{
}

GaussianProcess::GaussianProcess()
  : _virtual_params(torch::zeros({0, 0}, at::kDouble)),
    _penalty_points_std(torch::zeros({0, 0}, at::kDouble))
{
}

// ---- Link function -------------------------------------------------------

void
GaussianProcess::setLinkFunction(GPLinkFunctionType type, Real lb, Real ub)
{
  _link_type = type;
  _link_lb = lb;
  _link_ub = ub;
}

void
GaussianProcess::applyLinkTransform(torch::Tensor & data) const
{
  if (_link_type == GPLinkFunctionType::Identity)
    return;

  auto link = GPLinkFunction::build(_link_type, _link_lb, _link_ub);
  // Transform on a CPU copy via accessor, then write the result back to the
  // (possibly GPU-resident) input tensor.
  torch::Tensor cpu_data = LibtorchUtils::toCPUContiguous(data);
  auto data_accessor = cpu_data.accessor<Real, 2>();
  for (int64_t ii = 0; ii < cpu_data.size(0); ++ii)
    for (int64_t jj = 0; jj < cpu_data.size(1); ++jj)
    {
      const Real y = data_accessor[ii][jj];
      if (y <= link->lowerBound())
        mooseError("Training data value ",
                   y,
                   " is at or below the link function lower bound ",
                   link->lowerBound(),
                   ". Adjust the link lower bound or check training data.");
      if (y >= link->upperBound())
        mooseError("Training data value ",
                   y,
                   " is at or above the link function upper bound ",
                   link->upperBound(),
                   ". Adjust the link upper bound or check training data.");
      data_accessor[ii][jj] = link->forward(y);
    }
  data.copy_(cpu_data.to(data.options()));
}

Real
GaussianProcess::applyLink(Real y) const
{
  if (_link_type == GPLinkFunctionType::Identity)
    return y;
  return GPLinkFunction::build(_link_type, _link_lb, _link_ub)->forward(y);
}

Real
GaussianProcess::applyInvLink(Real z) const
{
  if (_link_type == GPLinkFunctionType::Identity)
    return z;
  return GPLinkFunction::build(_link_type, _link_lb, _link_ub)->inverse(z);
}

Real
GaussianProcess::invLinkDeriv(Real z) const
{
  if (_link_type == GPLinkFunctionType::Identity)
    return 1.0;
  return GPLinkFunction::build(_link_type, _link_lb, _link_ub)->inverseDeriv(z);
}

Real
GaussianProcess::logLinkJacobian(Real y) const
{
  if (_link_type == GPLinkFunctionType::Identity)
    return 0.0;
  return GPLinkFunction::build(_link_type, _link_lb, _link_ub)->logJacobian(y);
}

// ---- Constraint setters --------------------------------------------------

void
GaussianProcess::setDerivativeConstraints(const torch::Tensor & virtual_params,
                                          const std::vector<unsigned int> & deriv_dims,
                                          Real target_value,
                                          Real noise_variance)
{
  if ((unsigned)virtual_params.size(0) != deriv_dims.size())
    mooseError("virtual_params rows (",
               virtual_params.size(0),
               ") must match deriv_dims size (",
               deriv_dims.size(),
               ").");
  if (noise_variance <= 0.0)
    mooseError("derivative_noise_variance must be > 0 to keep the augmented covariance matrix "
               "positive-definite.");
  _virtual_params = virtual_params;
  _virtual_deriv_dims = deriv_dims;
  _virtual_target = target_value;
  _virtual_noise = noise_variance;
}

void
GaussianProcess::setPenaltyConstraints(const torch::Tensor & penalty_points_std,
                                       const std::vector<Real> & lower_bounds_std,
                                       const std::vector<Real> & upper_bounds_std,
                                       Real weight)
{
  if ((unsigned)penalty_points_std.size(0) != lower_bounds_std.size() ||
      (unsigned)penalty_points_std.size(0) != upper_bounds_std.size())
    mooseError("penalty_constraint_points rows must match the number of lower/upper bounds.");
  _penalty_points_std = penalty_points_std;
  _penalty_lower_std = lower_bounds_std;
  _penalty_upper_std = upper_bounds_std;
  _penalty_weight = weight;
}

void
GaussianProcess::initialize(CovarianceFunctionBase * covariance_function,
                            const std::vector<std::string> & params_to_tune,
                            const std::vector<Real> & min,
                            const std::vector<Real> & max)
{
  linkCovarianceFunction(covariance_function);
  generateTuningMap(params_to_tune, min, max);
}

void
GaussianProcess::linkCovarianceFunction(CovarianceFunctionBase * covariance_function)
{
  _covariance_function = covariance_function;
  _covar_type = _covariance_function->type();
  _covar_name = _covariance_function->name();
  _covariance_function->dependentCovarianceTypes(_dependent_covar_types);
  _dependent_covar_names = _covariance_function->dependentCovarianceNames();
  _num_outputs = _covariance_function->numOutputs();
}

void
GaussianProcess::setupCovarianceMatrix(const torch::Tensor & training_params,
                                       const torch::Tensor & training_data,
                                       const GPOptimizerOptions & opts)
{
  if (_virtual_params.size(0) > 0 && _num_outputs > 1)
    mooseError("Derivative observations (monotonicity constraints) are not supported for "
               "multi-output GPs. Use a single-output covariance function.");

  const auto options = doubleOptionsLike(training_params);
  const auto params = toOptions(training_params, options);
  const auto data = toOptions(training_data, options);

  mooseAssert(params.dim() == 2, "GaussianProcess training parameters must be rank-2.");
  mooseAssert(data.dim() == 2, "GaussianProcess training responses must be rank-2.");

  const auto num_samples = params.size(0);
  mooseAssert(data.size(0) == num_samples,
              "Training parameter and response sample counts must match.");
  mooseAssert(data.size(1) == _num_outputs,
              "Training response dimension does not match the covariance output dimension.");

  const bool batch_decision = opts.batch_size > 0 && (opts.batch_size <= num_samples);
  _batch_size = batch_decision ? opts.batch_size : num_samples;

  _hyperparam_map.clear();
  _covariance_function->buildHyperParamMap(_hyperparam_map);
  moveHyperParameters(_hyperparam_map, options);
  _covariance_function->loadHyperParamMap(_hyperparam_map);

  if (_tuning_data.size())
    tuneHyperParamsAdam(params, data, opts);

  const auto n_train = params.size(0);
  const auto n_virt = _virtual_params.size(0);

  if (n_virt == 0)
  {
    _covariance_function->computeCovarianceMatrix(_K, params, params, true);
    const auto flattened_tensor = flattenOutputData(data);

    // Compute the Cholesky decomposition and inverse action of the covariance matrix.
    setupStoredMatrices(flattened_tensor);
  }
  else
  {
    // Augmented covariance matrix incorporating virtual derivative observations.
    // K_aug = [ K_ff + sigma_n^2*I   K_fd             ]
    //         [ K_df                 K_dd + sigma_d^2*I ]
    const auto n_total = n_train + n_virt;
    _K = torch::zeros({n_total, n_total}, options);
    const auto virtual_params = toOptions(_virtual_params, options);

    // Top-left: K(X_train, X_train) + sigma_n^2*I (noise added by is_self_covariance=true)
    torch::Tensor K_ff; // computeCovarianceMatrix reassigns its output argument
    _covariance_function->computeCovarianceMatrix(K_ff, params, params, true);
    _K.narrow(0, 0, n_train).narrow(1, 0, n_train).copy_(K_ff);

    // Off-diagonal: K_fd and its transpose K_df
    for (unsigned int j = 0; j < n_virt; ++j)
    {
      const auto xd_j = virtual_params.narrow(0, j, 1);
      torch::Tensor K_fd_j;
      _covariance_function->computeCovarianceFD(K_fd_j, params, xd_j, _virtual_deriv_dims[j]);
      _K.narrow(0, 0, n_train).narrow(1, n_train + j, 1).copy_(K_fd_j);
      _K.narrow(0, n_train + j, 1).narrow(1, 0, n_train).copy_(K_fd_j.transpose(0, 1));
    }

    // Bottom-right: K_dd + sigma_d^2*I
    for (unsigned int i = 0; i < n_virt; ++i)
    {
      const auto xd_i = virtual_params.narrow(0, i, 1);
      for (unsigned int j = 0; j < n_virt; ++j)
      {
        const auto xd_j = virtual_params.narrow(0, j, 1);
        torch::Tensor K_dd_ij;
        _covariance_function->computeCovarianceDD(
            K_dd_ij, xd_i, xd_j, _virtual_deriv_dims[i], _virtual_deriv_dims[j]);
        _K.narrow(0, n_train + i, 1).narrow(1, n_train + j, 1).copy_(K_dd_ij);
      }
    }
    _K.narrow(0, n_train, n_virt).narrow(1, n_train, n_virt).diagonal().add_(_virtual_noise);

    // Augmented RHS: [y_train; v_virtual_target * ones]
    torch::Tensor augmented_data = torch::empty({n_total, 1}, options);
    augmented_data.narrow(0, 0, n_train).copy_(flattenOutputData(data));
    augmented_data.narrow(0, n_train, n_virt).fill_(_virtual_target);

    setupStoredMatrices(augmented_data);
  }
}

void
GaussianProcess::setupStoredMatrices(const torch::Tensor & input)
{
  _K_cho_decomp = torch::linalg_cholesky(_K);
  _K_results_solve = torch::cholesky_solve(input, _K_cho_decomp);
}

void
GaussianProcess::generateTuningMap(const std::vector<std::string> & params_to_tune,
                                   const std::vector<Real> & min_vector,
                                   const std::vector<Real> & max_vector)
{
  _num_tunable = 0;

  const bool upper_bounds_specified = min_vector.size();
  const bool lower_bounds_specified = max_vector.size();

  for (const auto param_i : index_range(params_to_tune))
  {
    const auto & hp = params_to_tune[param_i];
    if (_covariance_function->isTunable(hp))
    {
      unsigned int size;
      Real min;
      Real max;
      // Get size and default min/max
      const bool found = _covariance_function->getTuningData(hp, size, min, max);

      if (!found)
        ::mooseError("The covariance parameter ", hp, " could not be found!");

      // Check for overridden min/max
      min = lower_bounds_specified ? min_vector[param_i] : min;
      max = upper_bounds_specified ? max_vector[param_i] : max;
      // Save data in tuple
      _tuning_data[hp] = std::make_tuple(_num_tunable, size, min, max);
      _num_tunable += size;
    }
  }
}

void
GaussianProcess::standardizeParameters(torch::Tensor & data, bool keep_moments)
{
  if (!keep_moments)
    _param_standardizer.computeSet(data);
  _param_standardizer.getStandardized(data);
}

void
GaussianProcess::standardizeData(torch::Tensor & data, bool keep_moments)
{
  if (!keep_moments)
    _data_standardizer.computeSet(data);
  _data_standardizer.getStandardized(data);
}

void
GaussianProcess::tuneHyperParamsAdam(const torch::Tensor & training_params,
                                     const torch::Tensor & training_data,
                                     const GPOptimizerOptions & opts)
{
  const auto options = doubleOptionsLike(training_params);
  std::vector<Real> theta_values(_num_tunable, 0.0);

  mapToVec(_tuning_data, _hyperparam_map, theta_values);

  auto theta = torch::from_blob(theta_values.data(),
                                {static_cast<long>(_num_tunable)},
                                torch::TensorOptions().dtype(at::kDouble))
                   .clone()
                   .to(options.device());

  auto adam_options = torch::optim::AdamOptions(opts.learning_rate);
  adam_options.betas(std::make_tuple(opts.b1, opts.b2));
  adam_options.eps(opts.eps);
  // The legacy MOOSE shrink term is decoupled and not learning-rate-scaled, so it cannot be
  // represented by Adam's coupled weight_decay option.
  adam_options.weight_decay(0.0);
  torch::optim::Adam optimizer({theta}, adam_options);

  Real store_loss = 0.0;
  std::vector<Real> grad_values;
  const bool use_legacy_update = opts.optimizer_type == OptimizerType::LegacyAdam;

  const bool use_full_batch = _batch_size == static_cast<unsigned int>(training_params.size(0));
  // Preserve the existing deterministic shuffle sequence for mini-batches, but avoid rebuilding
  // shuffled full-batch tensors when the batch already contains every training sample.
  std::vector<unsigned int> v_sequence;
  if (!use_full_batch)
  {
    v_sequence.resize(training_params.size(0));
    std::iota(std::begin(v_sequence), std::end(v_sequence), 0);
  }
  if (opts.show_every_nth_iteration)
    Moose::out << "OPTIMIZING GP HYPER-PARAMETERS USING "
               << (use_legacy_update ? "legacy-compatible Adam" : "Adam") << std::endl;
  for (unsigned int ss = 0; ss < opts.num_iter; ++ss)
  {
    torch::Tensor inputs;
    torch::Tensor outputs;
    if (use_full_batch)
    {
      inputs = training_params;
      outputs = training_data;
    }
    else
    {
      MooseRandom generator;
      generator.seed(0, 1980);
      generator.saveState();
      MooseUtils::shuffle<unsigned int>(v_sequence, generator, 0);

      std::vector<int64_t> batch_indices_vec(v_sequence.begin(), v_sequence.begin() + _batch_size);
      auto batch_indices = torch::tensor(
          batch_indices_vec, torch::TensorOptions().dtype(torch::kLong).device(options.device()));
      inputs = torch::index_select(training_params, 0, batch_indices);
      outputs = torch::index_select(training_data, 0, batch_indices);
    }

    store_loss = getLoss(inputs, outputs);
    if (opts.show_every_nth_iteration && ((ss + 1) % opts.show_every_nth_iteration == 0))
      Moose::out << "Iteration: " << ss + 1 << " LOSS: " << store_loss << std::endl;

    grad_values = getGradient(inputs);
    auto grad = torch::from_blob(grad_values.data(),
                                 {static_cast<long>(_num_tunable)},
                                 torch::TensorOptions().dtype(at::kDouble))
                    .clone()
                    .to(options.device());
    optimizer.zero_grad();
    theta.mutable_grad() = grad;
    torch::Tensor theta_before_step;
    if (use_legacy_update)
      theta_before_step = theta.detach().clone();
    optimizer.step();

    {
      torch::NoGradGuard no_grad;
      if (use_legacy_update)
        theta -= opts.lambda * theta_before_step;
      for (auto iter = _tuning_data.begin(); iter != _tuning_data.end(); ++iter)
      {
        const auto first_index = std::get<0>(iter->second);
        const auto num_entries = std::get<1>(iter->second);
        const auto min_value = std::get<2>(iter->second);
        const auto max_value = std::get<3>(iter->second);
        theta.slice(0, first_index, first_index + num_entries).clamp_(min_value, max_value);
      }
    }

    const auto theta_export = LibtorchUtils::toCPUContiguous(theta);
    const auto * theta_data = theta_export.data_ptr<Real>();
    theta_values.assign(theta_data, theta_data + theta_export.numel());
    vecToMap(_tuning_data, _hyperparam_map, theta_values);
    _covariance_function->loadHyperParamMap(_hyperparam_map);
  }
  if (opts.show_every_nth_iteration)
  {
    Moose::out << "OPTIMIZED GP HYPER-PARAMETERS:" << std::endl;
    Moose::out << Moose::stringify(theta_values) << std::endl;
    Moose::out << "FINAL LOSS: " << store_loss << std::endl;
  }

  if (theta_values.size() > 0)
  {
    unsigned int count = 1;
    _length_scales.resize(_num_tunable - count);
    for (unsigned int i = 0; i < _num_tunable - count; ++i)
      _length_scales[i] = theta_values[i + 1];
  }
}

Real
GaussianProcess::getLoss(torch::Tensor & inputs, torch::Tensor & outputs)
{
  _covariance_function->computeCovarianceMatrix(_K, inputs, inputs, true);
  const auto flattened_data = flattenOutputData(outputs);

  setupStoredMatrices(flattened_data);

  Real log_likelihood = 0;
  log_likelihood +=
      -1 * torch::mm(torch::transpose(flattened_data, 0, 1), _K_results_solve).item<Real>();
  log_likelihood += -2.0 * torch::sum(torch::log(torch::diagonal(_K_cho_decomp))).item<Real>();
  log_likelihood -= flattened_data.size(0) * std::log(2 * M_PI);
  log_likelihood = -log_likelihood / 2;

  // Add penalty constraint terms (evaluated at constraint points using current batch model)
  const unsigned int n_penalty = _penalty_points_std.size(0);
  if (n_penalty > 0 && _penalty_weight > 0.0)
  {
    for (unsigned int c = 0; c < n_penalty; ++c)
    {
      const auto xc = _penalty_points_std.narrow(0, c, 1).to(inputs.options());
      torch::Tensor K_star; // computeCovarianceMatrix reassigns its output argument
      _covariance_function->computeCovarianceMatrix(K_star, inputs, xc, false);
      const Real mu_c = torch::mm(torch::transpose(K_star, 0, 1), _K_results_solve).item<Real>();

      const Real lb = _penalty_lower_std[c];
      const Real ub = _penalty_upper_std[c];
      if (lb > -std::numeric_limits<Real>::max() && mu_c < lb)
        log_likelihood += _penalty_weight * std::pow(lb - mu_c, 2);
      if (ub < std::numeric_limits<Real>::max() && mu_c > ub)
        log_likelihood += _penalty_weight * std::pow(mu_c - ub, 2);
    }
  }

  return log_likelihood;
}

std::vector<Real>
GaussianProcess::getGradient(torch::Tensor & inputs) const
{
  torch::Tensor dKdhp = torch::empty({_num_outputs * _batch_size, _num_outputs * _batch_size},
                                     doubleOptionsLike(inputs));
  std::vector<Real> grad_vec;
  grad_vec.resize(_num_tunable);

  // NLML gradient (existing computation)
  for (auto iter = _tuning_data.begin(); iter != _tuning_data.end(); ++iter)
  {
    std::string hyper_param_name = iter->first;
    const auto first_index = std::get<0>(iter->second);
    const auto num_entries = std::get<1>(iter->second);
    for (unsigned int ii = 0; ii < num_entries; ++ii)
    {
      const auto global_index = first_index + ii;
      _covariance_function->computedKdhyper(dKdhp, inputs, hyper_param_name, ii);
      const auto quadratic_form =
          torch::mm(torch::transpose(_K_results_solve, 0, 1), torch::mm(dKdhp, _K_results_solve))
              .item<Real>();
      const auto inverse_trace =
          torch::trace(torch::cholesky_solve(dKdhp, _K_cho_decomp)).item<Real>();
      grad_vec[global_index] = (inverse_trace - quadratic_form) / 2.0;
    }
  }

  // Penalty gradient contribution (approximate: only the dK_star/dtheta term)
  // d(penalty)/d(theta_i) ~= sum_c [violation_c * (-d(mu_c)/d(theta_i))]
  // where d(mu_c)/d(theta_i) ~= (dK_star/d(theta_i))^T * alpha_solve
  const unsigned int n_penalty = _penalty_points_std.size(0);
  if (n_penalty > 0 && _penalty_weight > 0.0)
  {
    torch::Tensor dK_star_dhp; // computedKdhyper_cross reassigns its output argument
    for (unsigned int c = 0; c < n_penalty; ++c)
    {
      const auto xc = _penalty_points_std.narrow(0, c, 1).to(inputs.options());
      torch::Tensor K_star; // computeCovarianceMatrix reassigns its output argument
      _covariance_function->computeCovarianceMatrix(K_star, inputs, xc, false);
      const Real mu_c = torch::mm(torch::transpose(K_star, 0, 1), _K_results_solve).item<Real>();

      const Real lb = _penalty_lower_std[c];
      const Real ub = _penalty_upper_std[c];

      Real slack = 0.0;
      int sign = 0;
      if (lb > -std::numeric_limits<Real>::max() && mu_c < lb)
      {
        slack = lb - mu_c;
        sign = -1; // penalty increases when mu_c decreases -> gradient pushes mu_c up
      }
      else if (ub < std::numeric_limits<Real>::max() && mu_c > ub)
      {
        slack = mu_c - ub;
        sign = 1; // penalty increases when mu_c increases -> gradient pushes mu_c down
      }

      if (sign == 0)
        continue;

      for (auto iter = _tuning_data.begin(); iter != _tuning_data.end(); ++iter)
      {
        const auto & hp_name = iter->first;
        const auto first_index = std::get<0>(iter->second);
        const auto num_entries = std::get<1>(iter->second);
        for (unsigned int ii = 0; ii < num_entries; ++ii)
        {
          const auto global_index = first_index + ii;
          _covariance_function->computedKdhyper_cross(dK_star_dhp, inputs, xc, hp_name, ii);
          const Real d_mu_c =
              torch::mm(torch::transpose(dK_star_dhp, 0, 1), _K_results_solve).item<Real>();
          // d(penalty)/d(theta_i) = 2*lambda*slack * sign * d(mu_c)/d(theta_i)
          grad_vec[global_index] += 2.0 * _penalty_weight * slack * sign * d_mu_c;
        }
      }
    }
  }

  return grad_vec;
}

void
GaussianProcess::mapToVec(
    const std::unordered_map<std::string, std::tuple<unsigned int, unsigned int, Real, Real>> &
        tuning_data,
    const HyperParameterMap & hyperparam_map,
    std::vector<Real> & vec) const
{
  for (auto iter : tuning_data)
  {
    const std::string & param_name = iter.first;
    const auto tensor_it = hyperparam_map.find(param_name);
    if (tensor_it == hyperparam_map.end())
      mooseError("The covariance parameter ", param_name, " could not be found!");

    const auto values = exportHyperParameter(tensor_it->second);
    const auto num_entries = std::get<1>(iter.second);
    mooseAssert(values.size() == num_entries,
                "Hyperparameter size does not match tuning metadata.");
    for (unsigned int ii = 0; ii < num_entries; ++ii)
      vec[std::get<0>(iter.second) + ii] = values[ii];
  }
}

void
GaussianProcess::vecToMap(
    const std::unordered_map<std::string, std::tuple<unsigned int, unsigned int, Real, Real>> &
        tuning_data,
    HyperParameterMap & hyperparam_map,
    const std::vector<Real> & vec) const
{
  for (auto iter : tuning_data)
  {
    const std::string & param_name = iter.first;
    const auto tensor_it = hyperparam_map.find(param_name);
    if (tensor_it == hyperparam_map.end())
      mooseError("The covariance parameter ", param_name, " could not be found!");

    const auto first_index = std::get<0>(iter.second);
    const auto num_entries = std::get<1>(iter.second);
    std::vector<Real> values(num_entries);
    for (unsigned int ii = 0; ii < num_entries; ++ii)
      values[ii] = vec[first_index + ii];

    updateHyperParameter(tensor_it->second, values, param_name);
  }
}

} // StochasticTools namespace

template <>
void
dataStore(std::ostream & stream, StochasticTools::GaussianProcess & gp_utils, void * context)
{
  dataStore(stream, gp_utils.hyperparamMap(), context);
  dataStore(stream, gp_utils.covarType(), context);
  dataStore(stream, gp_utils.covarName(), context);
  dataStore(stream, gp_utils.covarNumOutputs(), context);
  dataStore(stream, gp_utils.dependentCovarNames(), context);
  dataStore(stream, gp_utils.dependentCovarTypes(), context);
  dataStore(stream, gp_utils.K(), context);
  dataStore(stream, gp_utils.KResultsSolve(), context);
  dataStore(stream, gp_utils.KCholeskyDecomp(), context);
  dataStore(stream, gp_utils.paramStandardizer(), context);
  dataStore(stream, gp_utils.dataStandardizer(), context);
  // Link function
  int link_type_int = static_cast<int>(gp_utils.linkType());
  dataStore(stream, link_type_int, context);
  dataStore(stream, gp_utils.linkLb(), context);
  dataStore(stream, gp_utils.linkUb(), context);
  // Virtual derivative observations
  dataStore(stream, gp_utils.virtualParams(), context);
  dataStore(stream, gp_utils.virtualDerivDims(), context);
  dataStore(stream, gp_utils.virtualNoise(), context);
  dataStore(stream, gp_utils.virtualTarget(), context);
}

template <>
void
dataLoad(std::istream & stream, StochasticTools::GaussianProcess & gp_utils, void * context)
{
  dataLoad(stream, gp_utils.hyperparamMap(), context);
  dataLoad(stream, gp_utils.covarType(), context);
  dataLoad(stream, gp_utils.covarName(), context);
  dataLoad(stream, gp_utils.covarNumOutputs(), context);
  dataLoad(stream, gp_utils.dependentCovarNames(), context);
  dataLoad(stream, gp_utils.dependentCovarTypes(), context);
  dataLoad(stream, gp_utils.K(), context);
  dataLoad(stream, gp_utils.KResultsSolve(), context);
  dataLoad(stream, gp_utils.KCholeskyDecomp(), context);
  dataLoad(stream, gp_utils.paramStandardizer(), context);
  dataLoad(stream, gp_utils.dataStandardizer(), context);
  // Link function
  int link_type_int;
  dataLoad(stream, link_type_int, context);
  gp_utils.linkType() = static_cast<StochasticTools::GPLinkFunctionType>(link_type_int);
  dataLoad(stream, gp_utils.linkLb(), context);
  dataLoad(stream, gp_utils.linkUb(), context);
  // Virtual derivative observations
  dataLoad(stream, gp_utils.virtualParams(), context);
  dataLoad(stream, gp_utils.virtualDerivDims(), context);
  dataLoad(stream, gp_utils.virtualNoise(), context);
  dataLoad(stream, gp_utils.virtualTarget(), context);
}

#endif
