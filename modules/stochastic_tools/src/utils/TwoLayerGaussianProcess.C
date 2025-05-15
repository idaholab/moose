//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TwoLayerGaussianProcess.h"
#include "FEProblemBase.h"

#include <petsctao.h>
#include <petscdmda.h>

#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"

#include <cmath>

#include "MooseRandom.h"
#include "Shuffle.h"

#include "Normal.h"
#include "Uniform.h"
#include "Gamma.h"


namespace StochasticTools
{

TwoLayerGaussianProcess::TGPOptimizerOptions::TGPOptimizerOptions(const bool show_every_nth_iteration,
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

TwoLayerGaussianProcess::TwoLayerGaussianProcess() {}

void
TwoLayerGaussianProcess::initialize(CovarianceFunctionBase * covariance_function,
                            const std::vector<std::string> & params_to_tune,
                            const std::vector<Real> & min,
                            const std::vector<Real> & max)
{
  linkCovarianceFunction(covariance_function);
  generateTuningMap(params_to_tune, min, max);
}

void
TwoLayerGaussianProcess::linkCovarianceFunction(CovarianceFunctionBase * covariance_function)
{
  _covariance_function = covariance_function;
  _covar_type = _covariance_function->type();
  _covar_name = _covariance_function->name();
  _covariance_function->dependentCovarianceTypes(_dependent_covar_types);
  _dependent_covar_names = _covariance_function->dependentCovarianceNames();
  _num_outputs = _covariance_function->numOutputs();
}

void
TwoLayerGaussianProcess::setupCovarianceMatrix(const RealEigenMatrix & training_params,
                                       const RealEigenMatrix & training_data,
                                       const TGPOptimizerOptions & opts)
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
TwoLayerGaussianProcess::setupStoredMatrices(const RealEigenMatrix & input)
{
  _K_cho_decomp = _K.llt();
  _K_results_solve = _K_cho_decomp.solve(input);
}

void
TwoLayerGaussianProcess::generateTuningMap(const std::vector<std::string> & params_to_tune,
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
TwoLayerGaussianProcess::standardizeParameters(RealEigenMatrix & data, bool keep_moments)
{
  if (!keep_moments)
    _param_standardizer.computeSet(data);
  _param_standardizer.getStandardized(data);
}

void
TwoLayerGaussianProcess::standardizeData(RealEigenMatrix & data, bool keep_moments)
{
  if (!keep_moments)
    _data_standardizer.computeSet(data);
  _data_standardizer.getStandardized(data);
}

                                    
void
TwoLayerGaussianProcess::logl(const RealEigenMatrix & out_vec, const RealEigenMatrix & x1, const RealEigenMatrix & x2, Real noise, const RealEigenMatrix & lengthscale, 
          LogLResult & result, bool outer, bool scale) {
  int n = out_vec.rows();
  RealEigenMatrix K(x1.rows(), x2.rows());

  squared_exponential_covariance(x1, x2, 1, lengthscale, noise, K);
  Eigen::LLT<RealEigenMatrix> llt(K);
  RealEigenMatrix Mi = llt.solve(RealEigenMatrix::Identity(K.rows(), K.cols()));
  RealEigenMatrix L = llt.matrixL();
  Real ldet = 2 * L.diagonal().array().log().sum();

  RealEigenMatrix diff = out_vec;
  Real quadterm = (diff.transpose() * Mi * diff)(0,0);

  Real logl_val;
  if (outer) {
      logl_val = (-n * 0.5) * std::log(quadterm) - 0.5 * ldet;
  } else {
      logl_val = -0.5 * quadterm - 0.5 * ldet;
  }

  Real scale_val;
  if (scale) {
      scale_val = quadterm / n;
  } else {
      scale_val = NAN;
  }

  result.logl = logl_val;
  result.scale = scale_val; 
}

void
TwoLayerGaussianProcess::sampleNoise(const RealEigenMatrix & out_vec, const RealEigenMatrix & x1, const RealEigenMatrix & x2, Real noise_t, const RealEigenMatrix lengthscale, 
              Settings & settings, Real ll_prev, SampleNoiseResult & result) {


  Real alpha = settings.alpha.noise;
  Real beta = settings.beta.noise;
  Real l = settings.l;
  Real u = settings.u;

  MooseRandom generator;
  generator.seed(0, 1980);
  // Real check_seed = MooseRandom::rand();
  // std::cout << "check_seed0 is " << check_seed << std::endl;
  Real ru1 = MooseRandom::rand();
  Real ru = MooseRandom::rand();
  Real noise_star = Uniform::quantile(ru1, l * noise_t / u, u * noise_t / l);

  if (std::isnan(ll_prev)) {
    LogLResult ll_result;
    logl(out_vec, x1, x2, noise_t, lengthscale, ll_result, true, false);
    ll_prev = ll_result.logl;
  }
  Real lpost_threshold = ll_prev + std::log(Gamma::pdf(noise_t - 1.5e-8, alpha, beta)) + 
                            std::log(ru) - std::log(noise_t) + std::log(noise_star);

  Real ll_new;
  LogLResult ll_result;
  logl(out_vec, x1, x2, noise_star, lengthscale, ll_result, true, false);
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
TwoLayerGaussianProcess::sampleLengthscale(const RealEigenMatrix & out_vec, const RealEigenMatrix & x1, const RealEigenMatrix & x2, Real noise, const RealEigenMatrix & lengthscale_t,
              unsigned int i, Real alpha, Real beta, Real l, Real u, SampleLengthscaleResult & result, Real ll_prev) {

  RealEigenMatrix lengthscale_star = lengthscale_t;
  Real ru1 = MooseRandom::rand();
  Real ru = MooseRandom::rand();
  lengthscale_star(0, i) = Uniform::quantile(ru1, l * lengthscale_t(0,i) / u, u * lengthscale_t(0,i) / l);

  if (std::isnan(ll_prev)) {
    LogLResult ll_result;
    logl(out_vec, x1, x2, noise, lengthscale_t, ll_result, true, true);
    ll_prev = ll_result.logl;
  }
            
  Real lpost_threshold = ll_prev + std::log(Gamma::pdf(lengthscale_t(0,i), alpha, beta)) + 
                            std::log(ru) - std::log(lengthscale_t(0,i)) + std::log(lengthscale_star(0,i));

  Real ll_new;
  Real scale_new;
  LogLResult ll_result;
  logl(out_vec, x1, x2, noise, lengthscale_star, ll_result, true, true);
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
    result.scale = NAN;
  }
}

void
TwoLayerGaussianProcess::multiVariateNormalSampling(const RealEigenMatrix & mean,const RealEigenMatrix & cov, unsigned int n_dim, unsigned int n_draw, RealEigenMatrix & final_sample_matrix)
{
  Eigen::LLT<RealEigenMatrix> cho_decomp = cov.llt();
  RealEigenMatrix L = cho_decomp.matrixL();
  // std::cout << "matrix l is " << L.rows() << ", " << L.cols() << std::endl;
  RealEigenMatrix standard_sample_matrix(n_dim, n_draw);
  Real ru;
  
  Real standard_sample;
  for (unsigned int i = 0; i < n_dim; i++) {
    for (unsigned int j = 0; j < n_draw; j++) {
      ru = MooseRandom::rand();
      standard_sample = Normal::quantile(ru, 0, 1);
      standard_sample_matrix(i,j) = standard_sample;
    }
  }
  final_sample_matrix  = mean + L * standard_sample_matrix;
}

void TwoLayerGaussianProcess::sampleW(const RealEigenMatrix & out_vec, RealEigenMatrix & w_t, const RealEigenMatrix & w1, const RealEigenMatrix & w2, 
              const RealEigenMatrix & x1, const RealEigenMatrix & x2, Real noise, const RealEigenMatrix & lengthscale_y, const RealEigenMatrix & lengthscale_w,
              SampleWResult & result, Real ll_prev, const RealEigenMatrix & prior_mean) {

  if (std::isnan(ll_prev)) {
    LogLResult ll_result;
    logl(out_vec, w1, w2, noise, lengthscale_y, ll_result, true, true);
    ll_prev = ll_result.logl;
  }
  RealEigenMatrix w_prior(x1.rows(),1);
  Real ru1;
  Real ru2;
  Real ru3;
  Real ru;
  RealEigenMatrix K(x1.rows(), x2.rows());
  Real a;
  Real amin;
  Real amax;
  Real ll_threshold;
  bool accept;
  unsigned int count;
  RealEigenMatrix w_prev;
  LogLResult ll_result;
  Real new_logl;
  RealEigenMatrix lengthscale_w_i(1, x1.cols());

  for (unsigned int i=0; i < x1.cols(); i++){
  // for (unsigned int i=0; i< 1; i++){
    for (unsigned int h=0; h < x1.cols(); h++){
      lengthscale_w_i(0,h) = lengthscale_w(0,i);
    }

    squared_exponential_covariance(x1, x2, 1, lengthscale_w_i, noise, K);

    multiVariateNormalSampling(prior_mean.col(i), K, prior_mean.col(i).rows(), 1, w_prior);

    ru1 = MooseRandom::rand();
    a = Uniform::quantile(ru1, 0, 2 * M_PI);
    amin = a - 2 * M_PI;
    amax = a;
    ru2 = MooseRandom::rand();
    ru = Uniform::quantile(ru2, 0, 1);
    ll_threshold = ll_prev + std::log(ru);
    accept = false;
    count = 0;
    w_prev = w_t.col(i);

    while (!accept) {
      count += 1;
      w_t.col(i) = w_prev * std::cos(a) + w_prior * std::sin(a);
      logl(out_vec, w_t, w_t, noise, lengthscale_y, ll_result, true, false);
      new_logl = ll_result.logl;

      if (new_logl > ll_threshold) {
        ll_prev = new_logl;
        accept = true;
      } 
      else {
        if (a < 0) {
          amin = a;
        } else {
          amax = a;
        }
        ru3 = MooseRandom::rand();
        a = Uniform::quantile(ru3, amin, amax);
        if (count > 100) {
          break;
        }
      }
    }
  }
  result.w = w_t;
  result.ll = ll_prev;
}


void 
TwoLayerGaussianProcess::squared_exponential_covariance(const RealEigenMatrix &x1, 
                  const RealEigenMatrix &x2, 
                  Real scale, 
                  const RealEigenMatrix &lengthscale, 
                  Real noise, 
                  RealEigenMatrix &k)
{
  int n1 = x1.rows();
  int n2 = x2.rows();

  
  for (int i = 0; i < n1; i++) {
    for (int j = 0; j < n2; j++) {
      // Compute the scaled distance r_l(x1, x2)
      Eigen::RowVectorXd diff = (x1.row(i) - x2.row(j)).array() / lengthscale.row(0).array();
      Real r_l = std::sqrt(diff.squaredNorm());
      Real cov_val = scale * std::exp(-0.5 * r_l * r_l);
      if (i == j) {
          cov_val += noise;
      }
      k(i, j) = cov_val;
    }
  }
}

void
TwoLayerGaussianProcess::krig(const RealEigenMatrix & y, const RealEigenMatrix & x, const RealEigenMatrix & x_new,
                                   const RealEigenMatrix & lengthscale, Real noise, Real scale, bool cal_sigma,
                                   const RealEigenMatrix & prior_mean, const RealEigenMatrix & prior_mean_new,
                                   RealEigenMatrix & krig_mean, RealEigenMatrix & krig_sigma)
{
  RealEigenMatrix C(x.rows(), x.rows());
  RealEigenMatrix C_cross(x_new.rows(), x.rows());
  RealEigenMatrix C_new(x_new.rows(), x_new.rows());
  squared_exponential_covariance(x, x, scale, lengthscale, noise, C);
  squared_exponential_covariance(x_new, x, scale, lengthscale, noise, C_cross);

  Eigen::LLT<RealEigenMatrix> llt(C);
  RealEigenMatrix C_inv = llt.solve(RealEigenMatrix::Identity(C.rows(), C.cols()));

  krig_mean =  C_cross * C_inv * (y - prior_mean);


  if (cal_sigma) {
    RealEigenMatrix quad_term = C_cross * C_inv * C_cross.transpose();
    squared_exponential_covariance(x_new, x_new, scale, lengthscale, noise, C_new);
    krig_sigma = scale * (C_new - quad_term);
  }

}


void 
TwoLayerGaussianProcess::check_settings(Settings &settings) {
  settings.l = 1;
  settings.u = 2;

  settings.alpha.noise = 1.5;
  settings.beta.noise = 3.9;

  settings.alpha.lengthscale_w = 1.5;
  settings.alpha.lengthscale_y = 1.5;
  settings.beta.lengthscale_w = 3.9 / 4;
  settings.beta.lengthscale_y = 3.9 / 6;
}


void
TwoLayerGaussianProcess::tuneHyperParamsMcmc(const RealEigenMatrix & training_params,
                                     const RealEigenMatrix & training_data)

{ 
  RealEigenMatrix x = training_params;
  RealEigenMatrix y = training_data;

  unsigned int nmcmc = 10000;
  unsigned int burn = 8000;
  unsigned int thin = 2;

  Real noise_0 = 0.01;
  RealEigenMatrix lengthscale_y_0 = RealEigenMatrix::Constant(1, x.cols(), 0.5);
  RealEigenMatrix lengthscale_w_0 = RealEigenMatrix::Constant(1, x.cols(), 1);
  RealEigenMatrix w_0 = training_params;
  Real scale_0 = 1;

  Settings settings;
  check_settings(settings);

  Initial initial = {w_0, lengthscale_y_0, lengthscale_w_0, noise_0, scale_0};

 
  RealEigenMatrix noise(nmcmc, 1);
  noise(0,0) = initial.noise;
  RealEigenMatrix lengthscale_y(nmcmc, x.cols());
  lengthscale_y.row(0) = initial.lengthscale_y;
  RealEigenMatrix lengthscale_w(nmcmc, x.cols());
  lengthscale_w.row(0) = initial.lengthscale_w;
  std::vector<RealEigenMatrix> w(nmcmc);
  w[0] = initial.w;
  RealEigenMatrix scale(nmcmc, 1);
  scale(0,0) = initial.scale;
  RealEigenMatrix ll_store(nmcmc, x.cols());
  for (unsigned int j = 0; j < x.cols(); j++) {
    ll_store(0, j) = NAN;
  }
  Real ll = NAN;

  for (unsigned int j = 1; j < nmcmc; ++j) {
    MooseRandom generator;
    generator.seed(0, 1980);
    SampleNoiseResult sample_noise_result;
    sampleNoise(y, w[j-1], w[j-1], noise(j-1,0), lengthscale_y.row(j-1), settings, ll, sample_noise_result);
    noise(j,0) = sample_noise_result.noise;
    ll = sample_noise_result.ll;

    for (unsigned int i=0; i<x.cols(); i++){
      SampleLengthscaleResult sample_lengthscale_result;
      sampleLengthscale(y, w[j-1], w[j-1], noise(j,0), lengthscale_y.row(j-1), i, settings.alpha.lengthscale_y, settings.beta.lengthscale_y, settings.l,
                    settings.u, sample_lengthscale_result, ll);
      lengthscale_y(j,i) = sample_lengthscale_result.lengthscale;
      ll = sample_lengthscale_result.ll;
      ll_store(j,i) = ll;
      if (std::isnan(sample_lengthscale_result.scale)) {
        scale(j,0) = scale(j-1,0);
      }
      else {
        scale(j,0) = sample_lengthscale_result.scale;
      }
    }
    for (unsigned int i=0; i<x.cols(); i++){
      Real noise = 1.5e-8;
      SampleLengthscaleResult sample_lengthscale_w_result;
      sampleLengthscale(w[j-1].col(i), x, x, noise, lengthscale_w.row(j-1), i, settings.alpha.lengthscale_w, settings.beta.lengthscale_w, settings.l,
                    settings.u, sample_lengthscale_w_result, ll);
      lengthscale_w(j,i) = sample_lengthscale_w_result.lengthscale;
    }


    RealEigenMatrix prior_mean = RealEigenMatrix::Zero(x.rows(), x.cols());

    SampleWResult sample_w_result;
    sampleW(y, w[j-1], w[j-1], w[j-1], x, x, noise(j,0), lengthscale_y.row(j), lengthscale_w.row(j), sample_w_result, ll, prior_mean);
    w[j] = sample_w_result.w;
    ll = sample_w_result.ll;
    for (unsigned int i=0; i<x.cols(); i++){
     ll_store(j,i) = ll;
    }

  }


  unsigned int final_nmcmc = (nmcmc - burn) / thin;

  RealEigenMatrix noise_thinned(final_nmcmc, noise.cols());
  RealEigenMatrix lengthscale_y_thinned(final_nmcmc, lengthscale_y.cols());
  RealEigenMatrix lengthscale_w_thinned(final_nmcmc, lengthscale_w.cols());
  RealEigenMatrix scale_thinned(final_nmcmc, scale.cols());
  std::vector<RealEigenMatrix> w_thinned(final_nmcmc);


  unsigned int index = 0;
  for (unsigned int i = burn; i < 10000; i += thin) {
    noise_thinned.row(index) = noise.row(i);
    lengthscale_y_thinned.row(index) = lengthscale_y.row(i);
    lengthscale_w_thinned.row(index) = lengthscale_w.row(i);
    scale_thinned.row(index) = scale.row(i);
    w_thinned[index] = w[i];
    index++;
  }
  _noise = RealEigenMatrix::Constant(final_nmcmc, noise.cols(), 1e-6);
  _lengthscale_y = lengthscale_y_thinned;
  _lengthscale_w = lengthscale_w_thinned;
  _scale = scale_thinned;
  _w = w_thinned;
  _nmcmc = final_nmcmc;
  _x = x;
  _y = y;

}

void
TwoLayerGaussianProcess::tuneHyperParamsAdam(const RealEigenMatrix & training_params,
                                     const RealEigenMatrix & training_data,
                                     const TGPOptimizerOptions & opts)
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
    Moose::out << "OPTIMIZING TGP HYPER-PARAMETERS USING Adam" << std::endl;
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
    Moose::out << "OPTIMIZED TGP HYPER-PARAMETERS:" << std::endl;
    Moose::out << Moose::stringify(theta) << std::endl;
    Moose::out << "FINAL LOSS: " << store_loss << std::endl;
  }
}

Real
TwoLayerGaussianProcess::getLoss(RealEigenMatrix & inputs, RealEigenMatrix & outputs)
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
TwoLayerGaussianProcess::getGradient(RealEigenMatrix & inputs) const
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
TwoLayerGaussianProcess::mapToVec(
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
TwoLayerGaussianProcess::vecToMap(
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

// template <>
// void
// dataStore(std::ostream & stream, Eigen::LLT<RealEigenMatrix> & decomp, void * context)
// {
//   // Store the L matrix as opposed to the full matrix to avoid compounding
//   // roundoff error and decomposition error
//   RealEigenMatrix L(decomp.matrixL());
//   dataStore(stream, L, context);
// }

// template <>
// void
// dataLoad(std::istream & stream, Eigen::LLT<RealEigenMatrix> & decomp, void * context)
// {
//   RealEigenMatrix L;
//   dataLoad(stream, L, context);
//   decomp.compute(L * L.transpose());
// }

template <>
void
dataStore(std::ostream & stream, StochasticTools::TwoLayerGaussianProcess & tgp_utils, void * context)
{
  dataStore(stream, tgp_utils.hyperparamMap(), context);
  dataStore(stream, tgp_utils.hyperparamVectorMap(), context);
  dataStore(stream, tgp_utils.covarType(), context);
  dataStore(stream, tgp_utils.covarName(), context);
  dataStore(stream, tgp_utils.covarNumOutputs(), context);
  dataStore(stream, tgp_utils.dependentCovarNames(), context);
  dataStore(stream, tgp_utils.dependentCovarTypes(), context);
  dataStore(stream, tgp_utils.K(), context);
  dataStore(stream, tgp_utils.KResultsSolve(), context);
  dataStore(stream, tgp_utils.KCholeskyDecomp(), context);
  dataStore(stream, tgp_utils.paramStandardizer(), context);
  dataStore(stream, tgp_utils.dataStandardizer(), context);
}

template <>
void
dataLoad(std::istream & stream, StochasticTools::TwoLayerGaussianProcess & tgp_utils, void * context)
{
  dataLoad(stream, tgp_utils.hyperparamMap(), context);
  dataLoad(stream, tgp_utils.hyperparamVectorMap(), context);
  dataLoad(stream, tgp_utils.covarType(), context);
  dataLoad(stream, tgp_utils.covarName(), context);
  dataLoad(stream, tgp_utils.covarNumOutputs(), context);
  dataLoad(stream, tgp_utils.dependentCovarNames(), context);
  dataLoad(stream, tgp_utils.dependentCovarTypes(), context);
  dataLoad(stream, tgp_utils.K(), context);
  dataLoad(stream, tgp_utils.KResultsSolve(), context);
  dataLoad(stream, tgp_utils.KCholeskyDecomp(), context);
  dataLoad(stream, tgp_utils.paramStandardizer(), context);
  dataLoad(stream, tgp_utils.dataStandardizer(), context);
}
