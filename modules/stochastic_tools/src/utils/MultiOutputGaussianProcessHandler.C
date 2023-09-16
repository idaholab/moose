//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiOutputGaussianProcessHandler.h"
#include "FEProblemBase.h"

#include <petsctao.h>
#include <petscdmda.h>

#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"

#include <cmath>

#include "MooseRandom.h"
#include "Shuffle.h"

namespace StochasticTools
{

MultiOutputGaussianProcessHandler::GPOptimizerOptions::GPOptimizerOptions(
    const bool inp_show_optimization_details,
    const unsigned int inp_iter,
    const unsigned int inp_batch_size,
    const Real inp_learning_rate)
  : show_optimization_details(inp_show_optimization_details),
    iter(inp_iter),
    batch_size(inp_batch_size),
    learning_rate(inp_learning_rate)
{
}

void
MultiOutputGaussianProcessHandler::initialize(OutputCovarianceBase * output_covariance,
                                              CovarianceFunctionBase * covariance_function,
                                              const std::vector<std::string> params_to_tune,
                                              std::vector<Real> min,
                                              std::vector<Real> max)
{
  linkCovarianceFunction(output_covariance, covariance_function);
  generateTuningMap(params_to_tune, min, max);
}

void
MultiOutputGaussianProcessHandler::linkCovarianceFunction(
    OutputCovarianceBase * output_covariance, CovarianceFunctionBase * covariance_function)
{
  // For outputs
  _output_covariance = output_covariance;
  _output_covar_type = _output_covariance->type();

  // For inputs
  _covariance_function = covariance_function;
  _covar_type = _covariance_function->type();
}

void
MultiOutputGaussianProcessHandler::setupCovarianceMatrix(const RealEigenMatrix & training_params,
                                              const RealEigenMatrix & training_data,
                                              const GPOptimizerOptions & opts)
{
  const unsigned int batch_size = opts.batch_size > 0 ? opts.batch_size : training_params.rows();
  _K.resize(batch_size, batch_size);
  _B.resize(training_data.cols(), training_data.cols());
  _kappa.resize(batch_size * training_data.cols(), batch_size * training_data.cols());
  // Initializing all latent params internally because there would be too many to ask the user
  _num_tunable_out = _output_covariance->setupNumLatent(training_data.cols());
  _latent.assign(_num_tunable_out, 1.0);

  tuneHyperParamsAdam(training_params,
                      training_data,
                      opts.iter,
                      batch_size,
                      opts.learning_rate,
                      opts.show_optimization_details);

  _K.resize(training_params.rows(), training_params.rows());
  _covariance_function->computeCovarianceMatrix(_K, training_params, training_params, true);

  // Compute the Cholesky decomposition and inverse action of the covariance matrix
  setupStoredMatrices(training_data);

  _covariance_function->buildHyperParamMap(_hyperparam_map, _hyperparam_vec_map);
}

void
MultiOutputGaussianProcessHandler::setupStoredMatrices(const RealEigenMatrix & input)
{
  _kappa_cho_decomp = _kappa.llt();
  _kappa_results_solve = _kappa_cho_decomp.solve(input);
}

void
MultiOutputGaussianProcessHandler::generateTuningMap(const std::vector<std::string> params_to_tune,
                                          std::vector<Real> min_vector,
                                          std::vector<Real> max_vector)
{
  _num_tunable_inp = 0;

  bool upper_bounds_specified = false;
  bool lower_bounds_specified = false;
  if (min_vector.size())
    lower_bounds_specified = true;

  if (max_vector.size())
    upper_bounds_specified = true;

  for (unsigned int param_i = 0; param_i < params_to_tune.size(); ++param_i)
  {
    const auto & hp = params_to_tune[param_i];
    if (_covariance_function->isTunable(hp))
    {
      unsigned int size;
      Real min;
      Real max;
      // Get size and default min/max
      _covariance_function->getTuningData(hp, size, min, max);
      // Check for overridden min/max
      min = lower_bounds_specified ? min_vector[param_i] : min;
      max = upper_bounds_specified ? max_vector[param_i] : max;
      // Save data in tuple
      _tuning_data[hp] = std::make_tuple(_num_tunable_inp, size, min, max);
      _num_tunable_inp += size;
    }
  }
}

void
MultiOutputGaussianProcessHandler::standardizeParameters(RealEigenMatrix & data)
{
  _param_standardizer.computeSet(data);
  _param_standardizer.getStandardized(data);
}

void
MultiOutputGaussianProcessHandler::standardizeData(RealEigenMatrix & data)
{
  _data_standardizer.computeCovariance(data);
  _data_standardizer.getStandardizedCovariance(data);
}

void
MultiOutputGaussianProcessHandler::tuneHyperParamsAdam(const RealEigenMatrix & training_params,
                                            const RealEigenMatrix & training_data,
                                            unsigned int iter,
                                            const unsigned int & batch_size,
                                            const Real & learning_rate,
                                            const bool & show_optimization_details)
{
  libMesh::PetscVector<Number> theta(_tao_comm, _num_tunable_inp);
  _batch_size = batch_size;
  _covariance_function->buildHyperParamMap(_hyperparam_map, _hyperparam_vec_map);
  mapToPetscVec(_tuning_data, _hyperparam_map, _hyperparam_vec_map, theta);
  RealEigenMatrix vectorize_out;
  // Internal params for Adam; set to the recommended values in the paper
  Real b1 = 0.9;
  Real b2 = 0.999;
  Real eps = 1e-7;
  Real lambda = 1e-4;

  std::vector<Real> m0(_num_tunable_inp + _num_tunable_out, 0.0);
  std::vector<Real> v0(_num_tunable_inp + _num_tunable_out, 0.0);
  Real new_val;
  Real m_hat;
  Real v_hat;
  Real store_loss;
  std::vector<Real> grad1;

  // Initialize randomizer
  std::vector<unsigned int> v_sequence(training_params.rows());
  std::iota(std::begin(v_sequence), std::end(v_sequence), 0);
  RealEigenMatrix inputs(_batch_size, training_params.cols());
  RealEigenMatrix outputs(_batch_size, training_data.cols());
  if (show_optimization_details)
    Moose::out << "OPTIMIZING GP HYPER-PARAMETERS USING Adam" << std::endl;
  for (unsigned int ss = 0; ss < iter; ++ss)
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
    vectorize_out = outputs.transpose().reshaped(_batch_size * outputs.cols(), 1);
    store_loss = getLoss(inputs, vectorize_out);
    if (show_optimization_details && ss == 0)
      Moose::out << "INITIAL LOSS: " << store_loss << std::endl;
    grad1 = getGradient(inputs); // , vectorize_out
    for (unsigned int ii = 0; ii < _num_tunable_inp + _num_tunable_out; ++ii)
    {
      m0[ii] = b1 * m0[ii] + (1 - b1) * grad1[ii];
      v0[ii] = b2 * v0[ii] + (1 - b2) * grad1[ii] * grad1[ii];
      m_hat = m0[ii] / (1 - std::pow(b1, (ss + 1)));
      v_hat = v0[ii] / (1 - std::pow(b2, (ss + 1)));
      // new_val = theta(ii) - learning_rate * m_hat / (std::sqrt(v_hat) + eps);
      if (ii < _num_tunable_inp)
        new_val = theta(ii) - 1.0 * (learning_rate * m_hat / (std::sqrt(v_hat) + eps) + lambda * theta(ii));
      else
        new_val = _latent[ii - _num_tunable_inp] -
                  1.0 * (learning_rate * m_hat / (std::sqrt(v_hat) + eps) +
                         lambda * _latent[ii - _num_tunable_inp]);
      if (new_val < 0.01) // constrain params on the lower side
        new_val = 0.01;
      if (ii < _num_tunable_inp)
        theta.set(ii, new_val);
      else
        _latent[ii - _num_tunable_inp] = new_val;
    }
    petscVecToMap(_tuning_data, _hyperparam_map, _hyperparam_vec_map, theta);
    _covariance_function->loadHyperParamMap(_hyperparam_map, _hyperparam_vec_map);
  }
  if (show_optimization_details)
  {
    Moose::out << "OPTIMIZED GP HYPER-PARAMETERS:" << std::endl;
    theta.print();
    Moose::out << "FINAL LOSS: " << store_loss << std::endl;
  }
}

Real
MultiOutputGaussianProcessHandler::getLoss(RealEigenMatrix & inputs, RealEigenMatrix & outputs)
{
  _covariance_function->computeCovarianceMatrix(_K, inputs, inputs, true);
  _output_covariance->computeBCovarianceMatrix(_B, _latent);
  _output_covariance->computeFullCovarianceMatrix(_kappa, _B, _K);
  setupStoredMatrices(outputs);
  Real log_likelihood = 0;
  log_likelihood += -(outputs * _kappa_results_solve)(0, 0);
  log_likelihood += -std::log(std::pow(_K.determinant(), _B.rows()) * std::pow(_B.determinant(), _K.rows()));
  log_likelihood -= _batch_size * std::log(2 * M_PI);
  log_likelihood = -log_likelihood / 2;
  return log_likelihood;
}

std::vector<Real>
MultiOutputGaussianProcessHandler::getGradient(
    RealEigenMatrix & inputs) // , RealEigenMatrix & outputs
{
  RealEigenMatrix dKdhp(_batch_size, _batch_size);
  RealEigenMatrix B_grad(_B.rows(), _B.rows());
  RealEigenMatrix alpha = _kappa_results_solve * _kappa_results_solve.transpose();
  std::vector<Real> grad_vec;
  RealEigenMatrix kappa_grad;
  grad_vec.resize(_num_tunable_inp + _num_tunable_out);
  int count;
  count = 1;
  for (auto iter = _tuning_data.begin(); iter != _tuning_data.end(); ++iter)
  {
    std::string hyper_param_name = iter->first;
    for (unsigned int ii = 0; ii < std::get<1>(iter->second); ++ii)
    {
      _covariance_function->computedKdhyper(dKdhp, inputs, hyper_param_name, ii);
      _output_covariance->computeFullCovarianceMatrix(kappa_grad, _B, dKdhp);
      RealEigenMatrix tmp = alpha * kappa_grad - _kappa_cho_decomp.solve(kappa_grad);
      Real grad1 = -tmp.trace() / 2.0;
      if (hyper_param_name.compare("length_factor") == 0)
      {
        grad_vec[count] = grad1;
        ++count;
      }
      else
        grad_vec[0] = grad1;
    }
  }
  for (unsigned int ii = 0; ii < _num_tunable_out; ++ii)
  {
    _output_covariance->computeBGrad(B_grad, _latent, ii);
    _output_covariance->computeFullCovarianceMatrix(kappa_grad, B_grad, _K);
    RealEigenMatrix tmp = alpha * kappa_grad - _kappa_cho_decomp.solve(kappa_grad);
    Real grad1 = -tmp.trace() / 2.0;
    grad_vec[count] = grad1;
    ++count;
  }
  return grad_vec;
}

void
MultiOutputGaussianProcessHandler::mapToPetscVec(
    const std::unordered_map<std::string, std::tuple<unsigned int, unsigned int, Real, Real>> &
        tuning_data,
    const std::unordered_map<std::string, Real> & scalar_map,
    const std::unordered_map<std::string, std::vector<Real>> & vector_map,
    libMesh::PetscVector<Number> & petsc_vec)
{
  for (auto iter = tuning_data.begin(); iter != tuning_data.end(); ++iter)
  {
    std::string param_name = iter->first;
    const auto scalar_it = scalar_map.find(param_name);
    if (scalar_it != scalar_map.end())
      petsc_vec.set(std::get<0>(iter->second), scalar_it->second);
    else
    {
      const auto vector_it = vector_map.find(param_name);
      if (vector_it != vector_map.end())
        for (unsigned int ii = 0; ii < std::get<1>(iter->second); ++ii)
          petsc_vec.set(std::get<0>(iter->second) + ii, (vector_it->second)[ii]);
    }
  }
}

void
MultiOutputGaussianProcessHandler::petscVecToMap(
    const std::unordered_map<std::string, std::tuple<unsigned int, unsigned int, Real, Real>> &
        tuning_data,
    std::unordered_map<std::string, Real> & scalar_map,
    std::unordered_map<std::string, std::vector<Real>> & vector_map,
    const libMesh::PetscVector<Number> & petsc_vec)
{
  for (auto iter = tuning_data.begin(); iter != tuning_data.end(); ++iter)
  {
    std::string param_name = iter->first;
    if (scalar_map.find(param_name) != scalar_map.end())
      scalar_map[param_name] = petsc_vec(std::get<0>(iter->second));
    else if (vector_map.find(param_name) != vector_map.end())
      for (unsigned int ii = 0; ii < std::get<1>(iter->second); ++ii)
        vector_map[param_name][ii] = petsc_vec(std::get<0>(iter->second) + ii);
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
dataStore(std::ostream & stream,
          StochasticTools::MultiOutputGaussianProcessHandler & gp_utils,
          void * context)
{
  dataStore(stream, gp_utils.hyperparamMap(), context);
  dataStore(stream, gp_utils.hyperparamVectorMap(), context);
  dataStore(stream, gp_utils.covarType(), context);
  dataStore(stream, gp_utils.K(), context);
  dataStore(stream, gp_utils.outputCovarType(), context);
  dataStore(stream, gp_utils.B(), context);
  dataStore(stream, gp_utils.kappaResultsSolve(), context);
  // dataStore(stream, gp_utils.kappaCholeskyDecomp(), context);
  dataStore(stream, gp_utils.paramStandardizer(), context);
  dataStore(stream, gp_utils.dataStandardizer(), context);
}

template <>
void
dataLoad(std::istream & stream,
         StochasticTools::MultiOutputGaussianProcessHandler & gp_utils,
         void * context)
{
  dataLoad(stream, gp_utils.hyperparamMap(), context);
  dataLoad(stream, gp_utils.hyperparamVectorMap(), context);
  dataLoad(stream, gp_utils.covarType(), context);
  dataLoad(stream, gp_utils.K(), context);
  dataLoad(stream, gp_utils.outputCovarType(), context);
  dataLoad(stream, gp_utils.B(), context);
  dataLoad(stream, gp_utils.kappaResultsSolve(), context);
  // dataLoad(stream, gp_utils.kappaCholeskyDecomp(), context);
  dataLoad(stream, gp_utils.paramStandardizer(), context);
  dataLoad(stream, gp_utils.dataStandardizer(), context);
}
