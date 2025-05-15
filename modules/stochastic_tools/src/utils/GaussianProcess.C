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
                                                        const unsigned int tune_method,
                                                        const Real learning_rate,
                                                        const Real b1,
                                                        const Real b2,
                                                        const Real eps,
                                                        const Real lambda)
  : show_every_nth_iteration(show_every_nth_iteration),
    num_iter(num_iter),
    batch_size(batch_size),
    tune_method(tune_method),
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

  if (_tuning_data.size()) {
    if (opts.tune_method == 0) {
      tuneHyperParamsAdam(training_params, training_data, opts);
    }
    else if (opts.tune_method == 1) {
      tuneHyperParamsMcmc(training_params, training_data);
    }
    else {
      throw std::invalid_argument("Unsupported tune_method: must be 0 (Adam) or 1 (MCMC)");
    }
  }

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
GaussianProcess::logl(const RealEigenMatrix & out_vec, const RealEigenMatrix & x1, const RealEigenMatrix & lengthscale, 
          LogLResult & result, bool scale) {
  int n = out_vec.rows();
  RealEigenMatrix K(x1.rows(), x1.rows());

  std::vector<Real> theta1(_num_tunable, 0.0);

  theta1[0] = 1;
  for (unsigned int i = 0; i < _num_tunable-1; ++i) {
    theta1[i + 1] = lengthscale(0,i);
  }

  vecToMap(_tuning_data, _hyperparam_map, _hyperparam_vec_map, theta1);
  _covariance_function->loadHyperParamMap(_hyperparam_map, _hyperparam_vec_map);

  _covariance_function->computeCovarianceMatrix(_K, x1, x1, true);

  RealEigenMatrix Mi = _K.llt().solve(RealEigenMatrix::Identity(_K.rows(), _K.cols()));
  Real ldet = std::log(_K.determinant());

  RealEigenMatrix diff = out_vec;
  Real quadterm = (diff.transpose() * Mi * diff)(0,0);

  Real logl_val;
  logl_val = -0.5 * quadterm - 0.5 * ldet;
  
  Real scale_val;
  if (scale) {
      scale_val = quadterm / n;
  } else {
      scale_val = -999;
  }

  result.logl = logl_val;
  result.scale = scale_val; 
}

void
GaussianProcess::sampleNoise(const RealEigenMatrix & out_vec, const RealEigenMatrix & x1, Real noise_t, const RealEigenMatrix lengthscale, 
              Settings & settings, Real ll_prev, SampleNoiseResult & result) {
  Real alpha = settings.alpha.noise;
  Real beta = settings.beta.noise;
  Real l = settings.l;
  Real u = settings.u;
  Real ru1 = MooseRandom::rand();
  Real ru = MooseRandom::rand();
  Real noise_star = Uniform::quantile(ru1, l * noise_t / u, u * noise_t / l);

  if (ll_prev == -999) {
    LogLResult ll_result;
    logl(out_vec, x1, lengthscale, ll_result, false);
    ll_prev = ll_result.logl;
  }
  Real lpost_threshold = ll_prev + std::log(Gamma::pdf(noise_t - 1.5e-8, alpha, beta)) + 
                            std::log(ru) - std::log(noise_t) + std::log(noise_star);

  Real ll_new;
  LogLResult ll_result;
  logl(out_vec, x1, lengthscale, ll_result, false);
  ll_new = ll_result.logl;

  Real new_val = ll_new + std::log(Gamma::pdf(noise_star - 1.5e-8, alpha, beta));
  
  if (new_val > lpost_threshold) {
    result.noise = noise_star;
    result.ll = ll_new;
  } else {
    result.noise = noise_t;
    result.ll = ll_prev;
  }
}

void
GaussianProcess::sampleLengthscale(const RealEigenMatrix & out_vec, const RealEigenMatrix & x1, const RealEigenMatrix & lengthscale_t,
              unsigned int i, Settings & settings, SampleLengthscaleResult & result, Real ll_prev) {
    Real alpha = settings.alpha.noise;
    Real beta = settings.beta.noise;
    Real l = settings.l;
    Real u = settings.u;
    RealEigenMatrix lengthscale_star = lengthscale_t;
    Real ru1 = MooseRandom::rand();
    Real ru = MooseRandom::rand();
    lengthscale_star(0, i) = Uniform::quantile(ru1, l * lengthscale_t(0,i) / u, u * lengthscale_t(0,i) / l);

    if (ll_prev == -999) {
      LogLResult ll_result;
      logl(out_vec, x1, lengthscale_t, ll_result, true);
      ll_prev = ll_result.logl;
    }
              
    Real lpost_threshold = ll_prev + std::log(Gamma::pdf(lengthscale_t(0,i), alpha, beta)) + 
                             std::log(ru) - std::log(lengthscale_t(0,i)) + std::log(lengthscale_star(0,i));

    Real ll_new;
    Real scale_new;
    LogLResult ll_result;
    logl(out_vec, x1, lengthscale_star, ll_result, true);
    ll_new = ll_result.logl;
    scale_new = ll_result.scale;
      
    Real new_val = ll_new + std::log(Gamma::pdf(lengthscale_star(0,i), alpha, beta));
    if (new_val > lpost_threshold) {
      result.lengthscale = lengthscale_star(0,i);
      result.ll = ll_new;
      result.scale = scale_new;
    } else {
      result.lengthscale = lengthscale_t(0,i);
      result.ll = ll_prev;
      result.scale = -999;
    }
}


void 
GaussianProcess::check_settings(Settings &settings) {
  settings.l = 1;
  settings.u = 2;

  settings.alpha.noise = 1.5;
  settings.beta.noise = 3.9;

  settings.alpha.lengthscale = 1.5;
  settings.beta.lengthscale = 3.9 / 1.5;
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
  
  unsigned int nmcmc = 10000;
  unsigned int burn = 0;
  unsigned int thin = 2;

  Settings settings;
  check_settings(settings);

  Real noise_0 = 0.01;
  RealEigenMatrix lengthscale_0 = RealEigenMatrix::Constant(1, x.cols(), 0.5);
  Real scale_0 = 1;

  Initial initial = {lengthscale_0, noise_0, scale_0};


  RealEigenMatrix noise(nmcmc, 1);
  noise(0,0) = initial.noise;
  RealEigenMatrix lengthscale(nmcmc, x.cols());
  lengthscale.row(0) = initial.lengthscale;
  RealEigenMatrix scale(nmcmc, 1);
  scale(0,0) = initial.scale;
  RealEigenMatrix ll_store(nmcmc, x.cols());
  for (unsigned int j = 0; j < x.cols(); j++) {
    lengthscale_0(0, j) = -999;
  }
  Real ll = -999;
  
  for (unsigned int j = 1; j < nmcmc; ++j) {
    SampleNoiseResult sample_noise_result;
    sampleNoise(y, x, noise(j-1,0), lengthscale.row(j-1), settings, ll, sample_noise_result);

    noise(j,0) = sample_noise_result.noise;
    ll = sample_noise_result.ll;

    for (unsigned int i=0; i<x.cols(); i++){
      SampleLengthscaleResult sample_lengthscale_result;
      sampleLengthscale(y, x, lengthscale.row(j-1), i, settings, sample_lengthscale_result, ll);
      lengthscale(j,i) = sample_lengthscale_result.lengthscale;
      ll = sample_lengthscale_result.ll;
      ll_store(j,i) = ll;
      if (sample_lengthscale_result.scale == -999) {
        scale(j,0) = scale(j-1,0);
      }
      else {
        scale(j,0) = sample_lengthscale_result.scale;
      }
    }
  }


  unsigned int final_nmcmc = (nmcmc - burn) / thin;

  RealEigenMatrix noise_thinned(final_nmcmc, noise.cols());
  RealEigenMatrix lengthscale_thinned(final_nmcmc, lengthscale.cols());
  RealEigenMatrix scale_thinned(final_nmcmc, scale.cols());

  unsigned int index = 0;
  for (unsigned int i = burn; i < nmcmc; i += thin) {
    noise_thinned.row(index) = noise.row(i);
    lengthscale_thinned.row(index) = lengthscale.row(i);
    scale_thinned.row(index) = scale.row(i);
    index++;
  }


  Real sum_scale = 0;
  std::vector<Real> sum_lengthscale(_num_tunable - 1, 0.0);  


  for (unsigned int k = 0; k < final_nmcmc; k++) {
    sum_scale += scale_thinned(k, 0);
    for (unsigned int i = 0; i < _num_tunable-1; i++) {
      sum_lengthscale[i] += lengthscale_thinned(k, i);
    }
  }
  theta1[0] = sum_scale / final_nmcmc;
  for (unsigned int i = 0; i < _num_tunable-1; ++i) {
    theta1[i + 1] = sum_lengthscale[i] / final_nmcmc;
  }

  vecToMap(_tuning_data, _hyperparam_map, _hyperparam_vec_map, theta1);
  _covariance_function->loadHyperParamMap(_hyperparam_map, _hyperparam_vec_map);
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
