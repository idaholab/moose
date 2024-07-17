//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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

#include "MooseRandom.h"
#include "Shuffle.h"

#include "Uniform.h"
#include "Gamma.h"


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

GaussianProcess::GaussianProcess() {}

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
  const bool batch_decision = opts.batch_size > 0 && (opts.batch_size <= training_params.rows());
  _batch_size = batch_decision ? opts.batch_size : training_params.rows();
  _K.resize(_num_outputs * _batch_size, _num_outputs * _batch_size);

  if (_tuning_data.size())
    // tuneHyperParamsAdam(training_params, training_data, opts);
    tuneHyperParamsMcmc(training_params, training_data);

  _K.resize(training_params.rows() * training_data.cols(),
            training_params.rows() * training_data.cols());
  _covariance_function->computeCovarianceMatrix(_K, training_params, training_params, true);

  RealEigenMatrix flattened_data =
      training_data.reshaped(training_params.rows() * training_data.cols(), 1);

  // Compute the Cholesky decomposition and inverse action of the covariance matrix
  setupStoredMatrices(flattened_data);

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
GaussianProcess::logl(const RealEigenMatrix & out_vec, const RealEigenMatrix & x1, const RealEigenMatrix & x2, Real g, const RealEigenMatrix & theta, 
          LogLResult & result, bool outer, bool tau2) {
  int n = out_vec.rows();
  RealEigenMatrix K(x1.rows(), x2.rows());

  std::vector<Real> theta1(_num_tunable, 0.0);
  theta1[0] = 1;
  theta1[1] = theta(0,0);
  theta1[2] = theta(0,1);

  vecToMap(_tuning_data, _hyperparam_map, _hyperparam_vec_map, theta1);
  _covariance_function->loadHyperParamMap(_hyperparam_map, _hyperparam_vec_map);

  _covariance_function->computeCovarianceMatrix(_K, x1, x2, true);

  RealEigenMatrix Mi = _K.llt().solve(RealEigenMatrix::Identity(_K.rows(), _K.cols()));
  Real ldet = std::log(_K.determinant());

  RealEigenMatrix diff = out_vec;
  Real quadterm = (diff.transpose() * Mi * diff)(0,0);

  Real logl_val;
  if (outer) {
      logl_val = (-n * 0.5) * std::log(quadterm) - 0.5 * ldet;
  } else {
      logl_val = -0.5 * quadterm - 0.5 * ldet;
  }

  Real tau2_val;
  if (tau2) {
      tau2_val = quadterm / n;
  } else {
      tau2_val = NAN;
  }

  result.logl = logl_val;
  result.tau2 = tau2_val; 
}

void
GaussianProcess::sample_g(const RealEigenMatrix & out_vec, const RealEigenMatrix & x1, const RealEigenMatrix & x2, Real g_t, const RealEigenMatrix theta, 
              Real alpha, Real beta, Real l, Real u, Real ll_prev, SampleGResult & result) {

  Real ru1 = MooseRandom::rand();
  Real ru = MooseRandom::rand();
  Real g_star = Uniform::quantile(ru1, l * g_t / u, u * g_t / l);

  if (std::isnan(ll_prev)) {
    LogLResult ll_result;
    logl(out_vec, x1, x2, g_t, theta, ll_result, true, false);
    ll_prev = ll_result.logl;
  }
  Real lpost_threshold = ll_prev + std::log(Gamma::pdf(g_t - 1.5e-8, alpha, beta)) + 
                            std::log(ru) - std::log(g_t) + std::log(g_star);

  Real ll_new;
  LogLResult ll_result;
  logl(out_vec, x1, x2, g_star, theta, ll_result, true, false);
  ll_new = ll_result.logl;

  Real new_val = ll_new + std::log(Gamma::pdf(g_star - 1.5e-8, alpha, beta));
  
  if (new_val > lpost_threshold) {
    result.g = g_star;
    result.ll = ll_new;
  } else {
    result.g = g_t;
    result.ll = ll_prev;
  }
}

void
GaussianProcess::sample_theta(const RealEigenMatrix & out_vec, const RealEigenMatrix & x1, const RealEigenMatrix & x2, Real g, const RealEigenMatrix & theta_t,
              unsigned int i, Real alpha, Real beta, Real l, Real u, SampleThetaResult & result, Real ll_prev) {

    RealEigenMatrix theta_star = theta_t;
    Real ru1 = MooseRandom::rand();
    Real ru = MooseRandom::rand();
    theta_star(0, i) = Uniform::quantile(ru1, l * theta_t(0,i) / u, u * theta_t(0,i) / l);

    if (std::isnan(ll_prev)) {
      LogLResult ll_result;
      logl(out_vec, x1, x2, g, theta_t, ll_result, true, true);
      ll_prev = ll_result.logl;
    }
              
    Real lpost_threshold = ll_prev + std::log(Gamma::pdf(theta_t(0,i), alpha, beta)) + 
                             std::log(ru) - std::log(theta_t(0,i)) + std::log(theta_star(0,i));

    Real ll_new;
    Real tau2_new;
    LogLResult ll_result;
    logl(out_vec, x1, x2, g, theta_star, ll_result, true, true);
    ll_new = ll_result.logl;
    tau2_new = ll_result.tau2;
      
    Real new_val = ll_new + std::log(Gamma::pdf(theta_star(0,i), alpha, beta));
    if (new_val > lpost_threshold) {
      result.theta = theta_star(0,i);
      result.ll = ll_new;
      result.tau2 = tau2_new;
    } else {
      result.theta = theta_t(0,i);
      result.ll = ll_prev;
      result.tau2 = NAN;
    }
}


void 
GaussianProcess::check_settings(Settings &settings) {
  settings.l = 1;
  settings.u = 2;

  settings.alpha.g = 1.5;
  settings.beta.g = 3.9;

  settings.alpha.theta = 1.5;
  settings.beta.theta = 3.9 / 1.5;
}


void
GaussianProcess::tuneHyperParamsMcmc(const RealEigenMatrix & training_params,
                                     const RealEigenMatrix & training_data)
{ 
  std::vector<Real> theta1(_num_tunable, 0.0);
  _covariance_function->buildHyperParamMap(_hyperparam_map, _hyperparam_vec_map);

  mapToVec(_tuning_data, _hyperparam_map, _hyperparam_vec_map, theta1);

  RealEigenMatrix x = training_params;
  RealEigenMatrix y = training_data;
  
  MooseRandom generator;
  generator.seed(0, 1980);
  unsigned int nmcmc = 10000;

  Settings settings;
  check_settings(settings);

  Real g_0 = 0.01;
  RealEigenMatrix theta_0(1,x.cols());
  theta_0 << 0.5,0.5;
  Real tau_0 = 1;

  Initial initial = {theta_0, g_0, tau_0};

  Output out;
  out.x = x;
  out.y = y;
  out.nmcmc = nmcmc;
  out.initial = initial;
  out.settings = settings;
  RealEigenMatrix g(nmcmc, 1);
  g(0,0) = initial.g;
  RealEigenMatrix theta(nmcmc, x.cols());
  theta.row(0) = initial.theta;
  RealEigenMatrix tau2(nmcmc, 1);
  tau2(0,0) = initial.tau2;
  RealEigenMatrix ll_store(nmcmc, x.cols());
  ll_store.row(0) << NAN, NAN;
  Real ll = NAN;
  
  for (unsigned int j = 1; j < nmcmc; ++j) {
    SampleGResult sample_g_result;
    sample_g(y, x, x, g(j-1,0), theta.row(j-1), settings.alpha.g, settings.beta.g, settings.l, settings.u, ll, sample_g_result);

    g(j,0) = sample_g_result.g;
    ll = sample_g_result.ll;

    for (unsigned int i=0; i<x.cols(); i++){
      SampleThetaResult sample_theta_result;
      sample_theta(y, x, x, g(j,0), theta.row(j-1), i, settings.alpha.theta, settings.beta.theta, settings.l,
                  settings.u, sample_theta_result, ll);
      theta(j,i) = sample_theta_result.theta;
      ll = sample_theta_result.ll;
      ll_store(j,i) = ll;
      if (std::isnan(sample_theta_result.tau2)) {
        tau2(j,0) = tau2(j-1,0);
      }
      else {
        tau2(j,0) = sample_theta_result.tau2;
      }
    }
    
    Real sum_tau2 = 0;
    Real sum_theta0 = 0;
    Real sum_theta1 = 0;

    for (unsigned int k = 0; k < j+1; ++k) {
        sum_tau2 += tau2(k,0);
        sum_theta0 += theta(k,0);
        sum_theta1 += theta(k,1);
    }

    theta1[0] = sum_tau2 / j;
    theta1[1] = sum_theta0 / j;
    theta1[2] = sum_theta1 / j;

    vecToMap(_tuning_data, _hyperparam_map, _hyperparam_vec_map, theta1);
    _covariance_function->loadHyperParamMap(_hyperparam_map, _hyperparam_vec_map);
  }
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
        new_val = theta[global_index] - opts.learning_rate * m_hat / (std::sqrt(v_hat) + eps);

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
  return log_likelihood;
}

std::vector<Real>
GaussianProcess::getGradient(RealEigenMatrix & inputs) const
{
  RealEigenMatrix dKdhp(_batch_size, _batch_size);
  RealEigenMatrix alpha = _K_results_solve * _K_results_solve.transpose();
  std::vector<Real> grad_vec;
  grad_vec.resize(_num_tunable);
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
}
