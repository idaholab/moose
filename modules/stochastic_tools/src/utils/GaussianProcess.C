//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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

namespace StochasticTools
{

GaussianProcess::GPOptimizerOptions::GPOptimizerOptions(const bool show_every_nth_iteration,
                                                        const unsigned int num_iter,
                                                        const unsigned int batch_size,
                                                        const Real learning_rate,
                                                        const Real b1,
                                                        const Real b2,
                                                        const Real eps,
                                                        const Real lambda)
  : show_every_nth_iteration(show_every_nth_iteration),
    num_iter(num_iter),
    batch_size(batch_size),
    learning_rate(learning_rate),
    b1(b1),
    b2(b2),
    eps(eps),
    lambda(lambda)
{
}

GaussianProcess::GaussianProcess() : _virtual_params(0, 0), _penalty_points_std(0, 0) {}

// ---- Link function -------------------------------------------------------

void
GaussianProcess::setLinkFunction(GPLinkFunctionType type, Real lb, Real ub)
{
  _link_type = type;
  _link_lb = lb;
  _link_ub = ub;
}

void
GaussianProcess::applyLinkTransform(RealEigenMatrix & data) const
{
  if (_link_type == GPLinkFunctionType::Identity)
    return;

  auto link = GPLinkFunction::build(_link_type, _link_lb, _link_ub);
  for (int ii = 0; ii < data.rows(); ++ii)
    for (int jj = 0; jj < data.cols(); ++jj)
    {
      const Real y = data(ii, jj);
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
      data(ii, jj) = link->forward(y);
    }
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
GaussianProcess::setDerivativeConstraints(const RealEigenMatrix & virtual_params,
                                          const std::vector<unsigned int> & deriv_dims,
                                          Real target_value,
                                          Real noise_variance)
{
  if ((unsigned)virtual_params.rows() != deriv_dims.size())
    mooseError("virtual_params rows (",
               virtual_params.rows(),
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
GaussianProcess::setPenaltyConstraints(const RealEigenMatrix & penalty_points_std,
                                       const std::vector<Real> & lower_bounds_std,
                                       const std::vector<Real> & upper_bounds_std,
                                       Real weight)
{
  if ((unsigned)penalty_points_std.rows() != lower_bounds_std.size() ||
      (unsigned)penalty_points_std.rows() != upper_bounds_std.size())
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
GaussianProcess::setupCovarianceMatrix(const RealEigenMatrix & training_params,
                                       const RealEigenMatrix & training_data,
                                       const GPOptimizerOptions & opts)
{
  if (_virtual_params.rows() > 0 && _num_outputs > 1)
    mooseError("Derivative observations (monotonicity constraints) are not supported for "
               "multi-output GPs. Use a single-output covariance function.");

  const bool batch_decision = opts.batch_size > 0 && (opts.batch_size <= training_params.rows());
  _batch_size = batch_decision ? opts.batch_size : training_params.rows();
  _K.resize(_num_outputs * _batch_size, _num_outputs * _batch_size);

  if (_tuning_data.size())
    tuneHyperParamsAdam(training_params, training_data, opts);

  const unsigned int n_train = training_params.rows();
  const unsigned int n_outputs = training_data.cols();
  const unsigned int n_virt = _virtual_params.rows();

  if (n_virt == 0)
  {
    _K.resize(n_train * n_outputs, n_train * n_outputs);
    _covariance_function->computeCovarianceMatrix(_K, training_params, training_params, true);
    RealEigenMatrix flattened_data = training_data.reshaped(n_train * n_outputs, 1);
    setupStoredMatrices(flattened_data);
  }
  else
  {
    // Augmented covariance matrix incorporating virtual derivative observations.
    // K_aug = [ K_ff + sigma_n^2*I   K_fd             ]
    //         [ K_df                 K_dd + sigma_d^2*I ]
    const unsigned int n_total = n_train + n_virt;
    _K.resize(n_total, n_total);
    _K.setZero();

    // Top-left: K(X_train, X_train) + sigma_n^2*I (noise added by is_self_covariance=true)
    RealEigenMatrix K_ff(n_train, n_train);
    _covariance_function->computeCovarianceMatrix(K_ff, training_params, training_params, true);
    _K.topLeftCorner(n_train, n_train) = K_ff;

    // Off-diagonal: K_fd and its transpose K_df
    for (unsigned int j = 0; j < n_virt; ++j)
    {
      RealEigenMatrix xd_j = _virtual_params.row(j);
      RealEigenMatrix K_fd_j(n_train, 1);
      _covariance_function->computeCovarianceFD(
          K_fd_j, training_params, xd_j, _virtual_deriv_dims[j]);
      _K.block(0, n_train + j, n_train, 1) = K_fd_j;
      _K.block(n_train + j, 0, 1, n_train) = K_fd_j.transpose();
    }

    // Bottom-right: K_dd + sigma_d^2*I
    for (unsigned int i = 0; i < n_virt; ++i)
    {
      RealEigenMatrix xd_i = _virtual_params.row(i);
      for (unsigned int j = 0; j < n_virt; ++j)
      {
        RealEigenMatrix xd_j = _virtual_params.row(j);
        RealEigenMatrix K_dd_ij(1, 1);
        _covariance_function->computeCovarianceDD(
            K_dd_ij, xd_i, xd_j, _virtual_deriv_dims[i], _virtual_deriv_dims[j]);
        _K(n_train + i, n_train + j) = K_dd_ij(0, 0);
      }
      _K(n_train + i, n_train + i) += _virtual_noise;
    }

    // Augmented RHS: [y_train; v_virtual_target * ones]
    RealEigenMatrix augmented_data(n_total, 1);
    augmented_data.topRows(n_train) = training_data.reshaped(n_train, 1);
    augmented_data.bottomRows(n_virt).setConstant(_virtual_target);

    setupStoredMatrices(augmented_data);
  }

  _covariance_function->buildHyperParamMap(_hyperparam_map, _hyperparam_vec_map);
}

void
GaussianProcess::setupStoredMatrices(const RealEigenMatrix & input)
{
  _K_cho_decomp = _K.llt();
  _K_results_solve = _K_cho_decomp.solve(input);
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
GaussianProcess::standardizeParameters(RealEigenMatrix & data, bool keep_moments)
{
  if (!keep_moments)
    _param_standardizer.computeSet(data);
  _param_standardizer.getStandardized(data);
}

void
GaussianProcess::standardizeData(RealEigenMatrix & data, bool keep_moments)
{
  if (!keep_moments)
    _data_standardizer.computeSet(data);
  _data_standardizer.getStandardized(data);
}

void
GaussianProcess::tuneHyperParamsAdam(const RealEigenMatrix & training_params,
                                     const RealEigenMatrix & training_data,
                                     const GPOptimizerOptions & opts)
{
  std::vector<Real> theta(_num_tunable, 0.0);
  _covariance_function->buildHyperParamMap(_hyperparam_map, _hyperparam_vec_map);

  mapToVec(_tuning_data, _hyperparam_map, _hyperparam_vec_map, theta);

  // Internal params for Adam; set to the recommended values in the paper
  Real b1 = opts.b1;
  Real b2 = opts.b2;
  Real eps = opts.eps;
  static constexpr Real lambda = 1e-4;

  std::vector<Real> m0(_num_tunable, 0.0);
  std::vector<Real> v0(_num_tunable, 0.0);

  Real new_val;
  Real m_hat;
  Real v_hat;
  Real store_loss = 0.0;
  std::vector<Real> grad1;

  // Initialize randomizer
  std::vector<unsigned int> v_sequence(training_params.rows());
  std::iota(std::begin(v_sequence), std::end(v_sequence), 0);
  RealEigenMatrix inputs(_batch_size, training_params.cols());
  RealEigenMatrix outputs(_batch_size, training_data.cols());
  if (opts.show_every_nth_iteration)
    Moose::out << "OPTIMIZING GP HYPER-PARAMETERS USING Adam" << std::endl;
  for (unsigned int ss = 0; ss < opts.num_iter; ++ss)
  {
    // Shuffle data
    MooseRandom generator;
    generator.seed(0, 1980);
    generator.saveState();
    MooseUtils::shuffle<unsigned int>(v_sequence, generator, 0);
    for (unsigned int ii = 0; ii < _batch_size; ++ii)
    {
      for (unsigned int jj = 0; jj < training_params.cols(); ++jj)
        inputs(ii, jj) = training_params(v_sequence[ii], jj);

      for (unsigned int jj = 0; jj < training_data.cols(); ++jj)
        outputs(ii, jj) = training_data(v_sequence[ii], jj);
    }

    store_loss = getLoss(inputs, outputs);
    if (opts.show_every_nth_iteration && ((ss + 1) % opts.show_every_nth_iteration == 0))
      Moose::out << "Iteration: " << ss + 1 << " LOSS: " << store_loss << std::endl;
    grad1 = getGradient(inputs);
    for (auto iter = _tuning_data.begin(); iter != _tuning_data.end(); ++iter)
    {
      const auto first_index = std::get<0>(iter->second);
      const auto num_entries = std::get<1>(iter->second);
      for (unsigned int ii = 0; ii < num_entries; ++ii)
      {
        const auto global_index = first_index + ii;
        m0[global_index] = b1 * m0[global_index] + (1 - b1) * grad1[global_index];
        v0[global_index] =
            b2 * v0[global_index] + (1 - b2) * grad1[global_index] * grad1[global_index];
        m_hat = m0[global_index] / (1 - std::pow(b1, (ss + 1)));
        v_hat = v0[global_index] / (1 - std::pow(b2, (ss + 1)));
        new_val =
            theta[global_index] - 1.0 * (opts.learning_rate * m_hat / (std::sqrt(v_hat) + eps) +
                                         lambda * theta[global_index]);

        const auto min_value = std::get<2>(iter->second);
        const auto max_value = std::get<3>(iter->second);

        theta[global_index] = std::min(std::max(new_val, min_value), max_value);
      }
    }
    vecToMap(_tuning_data, _hyperparam_map, _hyperparam_vec_map, theta);
    _covariance_function->loadHyperParamMap(_hyperparam_map, _hyperparam_vec_map);
  }
  if (opts.show_every_nth_iteration)
  {
    Moose::out << "OPTIMIZED GP HYPER-PARAMETERS:" << std::endl;
    Moose::out << Moose::stringify(theta) << std::endl;
    Moose::out << "FINAL LOSS: " << store_loss << std::endl;
  }

  if (theta.size() > 0)
  {
    unsigned int count = 1;
    _length_scales.resize(_num_tunable - count);
    for (unsigned int i = 0; i < _num_tunable - count; ++i)
      _length_scales[i] = theta[i + 1];
  }
}

Real
GaussianProcess::getLoss(RealEigenMatrix & inputs, RealEigenMatrix & outputs)
{
  _covariance_function->computeCovarianceMatrix(_K, inputs, inputs, true);

  RealEigenMatrix flattened_data = outputs.reshaped(outputs.rows() * outputs.cols(), 1);

  setupStoredMatrices(flattened_data);

  Real log_likelihood = 0;
  log_likelihood += -(flattened_data.transpose() * _K_results_solve)(0, 0);
  log_likelihood += -std::log(_K.determinant());
  log_likelihood -= _batch_size * std::log(2 * M_PI);
  log_likelihood = -log_likelihood / 2;

  // Add penalty constraint terms (evaluated at constraint points using current batch model)
  const unsigned int n_penalty = _penalty_points_std.rows();
  if (n_penalty > 0 && _penalty_weight > 0.0)
  {
    for (unsigned int c = 0; c < n_penalty; ++c)
    {
      RealEigenMatrix xc = _penalty_points_std.row(c);
      RealEigenMatrix K_star(_batch_size, 1);
      _covariance_function->computeCovarianceMatrix(K_star, inputs, xc, false);
      const Real mu_c = (K_star.transpose() * _K_results_solve)(0, 0);

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
GaussianProcess::getGradient(RealEigenMatrix & inputs) const
{
  RealEigenMatrix dKdhp(_batch_size, _batch_size);
  RealEigenMatrix alpha = _K_results_solve * _K_results_solve.transpose();
  std::vector<Real> grad_vec(_num_tunable, 0.0);

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
      RealEigenMatrix tmp = alpha * dKdhp - _K_cho_decomp.solve(dKdhp);
      grad_vec[global_index] = -tmp.trace() / 2.0;
    }
  }

  // Penalty gradient contribution (approximate: only the dK_star/dtheta term)
  // d(penalty)/d(theta_i) ≈ sum_c [violation_c * (-d(mu_c)/d(theta_i))]
  // where d(mu_c)/d(theta_i) ≈ (dK_star/d(theta_i))^T * alpha_solve
  const unsigned int n_penalty = _penalty_points_std.rows();
  if (n_penalty > 0 && _penalty_weight > 0.0)
  {
    RealEigenMatrix dK_star_dhp(_batch_size, 1);
    for (unsigned int c = 0; c < n_penalty; ++c)
    {
      RealEigenMatrix xc = _penalty_points_std.row(c);
      RealEigenMatrix K_star(_batch_size, 1);
      _covariance_function->computeCovarianceMatrix(K_star, inputs, xc, false);
      const Real mu_c = (K_star.transpose() * _K_results_solve)(0, 0);

      const Real lb = _penalty_lower_std[c];
      const Real ub = _penalty_upper_std[c];

      Real slack = 0.0;
      int sign = 0;
      if (lb > -std::numeric_limits<Real>::max() && mu_c < lb)
      {
        slack = lb - mu_c;
        sign = -1; // penalty increases when mu_c decreases → gradient pushes mu_c up
      }
      else if (ub < std::numeric_limits<Real>::max() && mu_c > ub)
      {
        slack = mu_c - ub;
        sign = 1; // penalty increases when mu_c increases → gradient pushes mu_c down
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
          const Real d_mu_c = (dK_star_dhp.transpose() * _K_results_solve)(0, 0);
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
    const std::unordered_map<std::string, Real> & scalar_map,
    const std::unordered_map<std::string, std::vector<Real>> & vector_map,
    std::vector<Real> & vec) const
{
  for (auto iter : tuning_data)
  {
    const std::string & param_name = iter.first;
    const auto scalar_it = scalar_map.find(param_name);
    if (scalar_it != scalar_map.end())
      vec[std::get<0>(iter.second)] = scalar_it->second;
    else
    {
      const auto vector_it = vector_map.find(param_name);
      if (vector_it != vector_map.end())
        for (unsigned int ii = 0; ii < std::get<1>(iter.second); ++ii)
          vec[std::get<0>(iter.second) + ii] = (vector_it->second)[ii];
    }
  }
}

void
GaussianProcess::vecToMap(
    const std::unordered_map<std::string, std::tuple<unsigned int, unsigned int, Real, Real>> &
        tuning_data,
    std::unordered_map<std::string, Real> & scalar_map,
    std::unordered_map<std::string, std::vector<Real>> & vector_map,
    const std::vector<Real> & vec) const
{
  for (auto iter : tuning_data)
  {
    const std::string & param_name = iter.first;
    if (scalar_map.find(param_name) != scalar_map.end())
      scalar_map[param_name] = vec[std::get<0>(iter.second)];
    else if (vector_map.find(param_name) != vector_map.end())
      for (unsigned int ii = 0; ii < std::get<1>(iter.second); ++ii)
        vector_map[param_name][ii] = vec[std::get<0>(iter.second) + ii];
  }
}

} // StochasticTools namespace

template <>
void
dataStore(std::ostream & stream, Eigen::LLT<RealEigenMatrix> & decomp, void * context)
{
  // Store the L matrix as opposed to the full matrix to avoid compounding
  // roundoff error and decomposition error
  RealEigenMatrix L(decomp.matrixL());
  dataStore(stream, L, context);
}

template <>
void
dataLoad(std::istream & stream, Eigen::LLT<RealEigenMatrix> & decomp, void * context)
{
  RealEigenMatrix L;
  dataLoad(stream, L, context);
  decomp.compute(L * L.transpose());
}

template <>
void
dataStore(std::ostream & stream, StochasticTools::GaussianProcess & gp_utils, void * context)
{
  dataStore(stream, gp_utils.hyperparamMap(), context);
  dataStore(stream, gp_utils.hyperparamVectorMap(), context);
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
  dataLoad(stream, gp_utils.hyperparamVectorMap(), context);
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
