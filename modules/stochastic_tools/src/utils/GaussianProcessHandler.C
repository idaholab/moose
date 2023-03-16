//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GaussianProcessHandler.h"
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

GaussianProcessHandler::GPOptimizerOptions::GPOptimizerOptions(
    const MooseEnum & inp_opt_type,
    const std::string & inp_tao_options,
    const bool inp_show_optimization_details,
    const unsigned int inp_iter_adam,
    const unsigned int inp_batch_size,
    const Real inp_learning_rate_adam)
  : opt_type(inp_opt_type),
    tao_options(inp_tao_options),
    show_optimization_details(inp_show_optimization_details),
    iter_adam(inp_iter_adam),
    batch_size(inp_batch_size),
    learning_rate_adam(inp_learning_rate_adam)
{
}

GaussianProcessHandler::GaussianProcessHandler() : _tao_comm(MPI_COMM_SELF) {}

void
GaussianProcessHandler::initialize(CovarianceFunctionBase * covariance_function,
                                   const std::vector<std::string> params_to_tune,
                                   std::vector<Real> min,
                                   std::vector<Real> max)
{
  linkCovarianceFunction(covariance_function);
  generateTuningMap(params_to_tune, min, max);
}

void
GaussianProcessHandler::linkCovarianceFunction(CovarianceFunctionBase * covariance_function)
{
  _covariance_function = covariance_function;
  _covar_type = _covariance_function->type();
}

void
GaussianProcessHandler::setupCovarianceMatrix(const RealEigenMatrix & training_params,
                                              const RealEigenMatrix & training_data,
                                              const GPOptimizerOptions & opts)
{
  const unsigned int batch_size = opts.batch_size > 0 ? opts.batch_size : training_params.rows();
  _K.resize(batch_size, batch_size);

  if (opts.opt_type == "tao")
  {
    if (tuneHyperParamsTAO(
            training_params, training_data, opts.tao_options, opts.show_optimization_details))
      ::mooseError("PETSc/TAO error in hyperparameter tuning.");
  }
  else if (opts.opt_type == "adam")
    tuneHyperParamsAdam(training_params,
                        training_data,
                        opts.iter_adam,
                        batch_size,
                        opts.learning_rate_adam,
                        opts.show_optimization_details);

  _K.resize(training_params.rows(), training_params.rows());
  _covariance_function->computeCovarianceMatrix(_K, training_params, training_params, true);

  // Compute the Cholesky decomposition and inverse action of the covariance matrix
  setupStoredMatrices(training_data);

  _covariance_function->buildHyperParamMap(_hyperparam_map, _hyperparam_vec_map);
}

void
GaussianProcessHandler::setupStoredMatrices(const RealEigenMatrix & input)
{
  _K_cho_decomp = _K.llt();
  _K_results_solve = _K_cho_decomp.solve(input);
}

void
GaussianProcessHandler::generateTuningMap(const std::vector<std::string> params_to_tune,
                                          std::vector<Real> min_vector,
                                          std::vector<Real> max_vector)
{
  _num_tunable = 0;

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
      _tuning_data[hp] = std::make_tuple(_num_tunable, size, min, max);
      _num_tunable += size;
    }
  }
}

void
GaussianProcessHandler::standardizeParameters(RealEigenMatrix & data, bool keep_moments)
{
  if (!keep_moments)
    _param_standardizer.computeSet(data);
  _param_standardizer.getStandardized(data);
}

void
GaussianProcessHandler::standardizeData(RealEigenMatrix & data, bool keep_moments)
{
  if (!keep_moments)
    _data_standardizer.computeSet(data);
  _data_standardizer.getStandardized(data);
}

PetscErrorCode
GaussianProcessHandler::tuneHyperParamsTAO(const RealEigenMatrix & training_params,
                                           const RealEigenMatrix & training_data,
                                           std::string tao_options,
                                           bool show_optimization_details)
{
  PetscErrorCode ierr;
  Tao tao;

  _training_params = &training_params;
  _training_data = &training_data;

  // Setup Tao optimization problem
  ierr = TaoCreate(_tao_comm.get(), &tao);
  CHKERRQ(ierr);
  ierr = PetscOptionsSetValue(NULL, "-tao_type", "bncg");
  CHKERRQ(ierr);
  ierr = PetscOptionsInsertString(NULL, tao_options.c_str());
  CHKERRQ(ierr);
  ierr = TaoSetFromOptions(tao);
  CHKERRQ(ierr);

  // Define petsc vector to hold tunable hyper-params
  libMesh::PetscVector<Number> theta(_tao_comm, _num_tunable);
  ierr = formInitialGuessTAO(theta.vec());
  CHKERRQ(ierr);
#if !PETSC_VERSION_LESS_THAN(3, 17, 0)
  ierr = TaoSetSolution(tao, theta.vec());
#else
  ierr = TaoSetInitialVector(tao, theta.vec());
#endif
  CHKERRQ(ierr);

  // Get Hyperparameter bounds.
  libMesh::PetscVector<Number> lower(_tao_comm, _num_tunable);
  libMesh::PetscVector<Number> upper(_tao_comm, _num_tunable);
  buildHyperParamBoundsTAO(lower, upper);
  CHKERRQ(ierr);
  ierr = TaoSetVariableBounds(tao, lower.vec(), upper.vec());
  CHKERRQ(ierr);

  // Set Objective and Gradient Callback
#if !PETSC_VERSION_LESS_THAN(3, 17, 0)
  ierr = TaoSetObjectiveAndGradient(tao, NULL, formFunctionGradientWrapper, (void *)this);
#else
  ierr = TaoSetObjectiveAndGradientRoutine(tao, formFunctionGradientWrapper, (void *)this);
#endif
  CHKERRQ(ierr);

  // Solve
  ierr = TaoSolve(tao);
  CHKERRQ(ierr);
  //
  if (show_optimization_details)
  {
    ierr = TaoView(tao, PETSC_VIEWER_STDOUT_WORLD);
    theta.print();
  }

  _covariance_function->loadHyperParamMap(_hyperparam_map, _hyperparam_vec_map);

  ierr = TaoDestroy(&tao);
  CHKERRQ(ierr);

  return 0;
}

PetscErrorCode
GaussianProcessHandler::formInitialGuessTAO(Vec theta_vec)
{
  libMesh::PetscVector<Number> theta(theta_vec, _tao_comm);
  _covariance_function->buildHyperParamMap(_hyperparam_map, _hyperparam_vec_map);
  mapToPetscVec(_tuning_data, _hyperparam_map, _hyperparam_vec_map, theta);
  return 0;
}

void
GaussianProcessHandler::buildHyperParamBoundsTAO(libMesh::PetscVector<Number> & theta_l,
                                                 libMesh::PetscVector<Number> & theta_u) const
{
  for (auto iter = _tuning_data.begin(); iter != _tuning_data.end(); ++iter)
  {
    for (unsigned int ii = 0; ii < std::get<1>(iter->second); ++ii)
    {
      theta_l.set(std::get<0>(iter->second) + ii, std::get<2>(iter->second));
      theta_u.set(std::get<0>(iter->second) + ii, std::get<3>(iter->second));
    }
  }
}

PetscErrorCode
GaussianProcessHandler::formFunctionGradientWrapper(
    Tao tao, Vec theta_vec, PetscReal * f, Vec grad_vec, void * ptr)
{
  GaussianProcessHandler * GP_ptr = (GaussianProcessHandler *)ptr;
  GP_ptr->formFunctionGradient(tao, theta_vec, f, grad_vec);
  return 0;
}

void
GaussianProcessHandler::formFunctionGradient(Tao /*tao*/,
                                             Vec theta_vec,
                                             PetscReal * f,
                                             Vec grad_vec)
{
  libMesh::PetscVector<Number> theta(theta_vec, _tao_comm);
  libMesh::PetscVector<Number> grad(grad_vec, _tao_comm);

  petscVecToMap(_tuning_data, _hyperparam_map, _hyperparam_vec_map, theta);
  _covariance_function->loadHyperParamMap(_hyperparam_map, _hyperparam_vec_map);
  _covariance_function->computeCovarianceMatrix(_K, *_training_params, *_training_params, true);
  setupStoredMatrices(*_training_data);

  // testing auto tuning
  RealEigenMatrix dKdhp(_training_params->rows(), _training_params->rows());
  RealEigenMatrix alpha = _K_results_solve * _K_results_solve.transpose();
  for (auto iter = _tuning_data.begin(); iter != _tuning_data.end(); ++iter)
  {
    std::string hyper_param_name = iter->first;
    for (unsigned int ii = 0; ii < std::get<1>(iter->second); ++ii)
    {
      _covariance_function->computedKdhyper(dKdhp, *_training_params, hyper_param_name, ii);
      RealEigenMatrix tmp = alpha * dKdhp - _K_cho_decomp.solve(dKdhp);
      grad.set(std::get<0>(iter->second) + ii, -tmp.trace() / 2.0);
    }
  }
  Real log_likelihood = 0;
  log_likelihood += -(_training_data->transpose() * _K_results_solve)(0, 0);
  log_likelihood += -std::log(_K.determinant());
  log_likelihood += -_training_data->rows() * std::log(2 * M_PI);
  log_likelihood = -log_likelihood / 2;
  *f = log_likelihood;
}

void
GaussianProcessHandler::tuneHyperParamsAdam(const RealEigenMatrix & training_params,
                                            const RealEigenMatrix & training_data,
                                            unsigned int iter,
                                            const unsigned int & batch_size,
                                            const Real & learning_rate,
                                            const bool & show_optimization_details)
{
  libMesh::PetscVector<Number> theta(_tao_comm, _num_tunable);
  _batch_size = batch_size;
  _covariance_function->buildHyperParamMap(_hyperparam_map, _hyperparam_vec_map);
  mapToPetscVec(_tuning_data, _hyperparam_map, _hyperparam_vec_map, theta);
  Real b1;
  Real b2;
  Real eps;
  // Internal params for Adam; set to the recommended values in the paper
  b1 = 0.9;
  b2 = 0.999;
  eps = 1e-7;
  std::vector<Real> m0(_num_tunable, 0.0);
  std::vector<Real> v0(_num_tunable, 0.0);

  Real new_val;
  Real m_hat;
  Real v_hat;
  Real store_loss;
  std::vector<Real> grad1;

  // Initialize randomizer
  std::vector<unsigned int> v_sequence(training_params.rows());
  std::iota(std::begin(v_sequence), std::end(v_sequence), 0);
  RealEigenMatrix inputs(_batch_size, training_params.cols());
  RealEigenMatrix outputs(_batch_size, 1);
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
      outputs(ii, 0) = training_data(v_sequence[ii], 0);
    }

    store_loss = getLossAdam(inputs, outputs);
    if (show_optimization_details && ss == 0)
      Moose::out << "INITIAL LOSS: " << store_loss << std::endl;
    grad1 = getGradientAdam(inputs);
    for (unsigned int ii = 0; ii < _num_tunable; ++ii)
    {
      m0[ii] = b1 * m0[ii] + (1 - b1) * grad1[ii];
      v0[ii] = b2 * v0[ii] + (1 - b2) * grad1[ii] * grad1[ii];
      m_hat = m0[ii] / (1 - std::pow(b1, (ss + 1)));
      v_hat = v0[ii] / (1 - std::pow(b2, (ss + 1)));
      new_val = theta(ii) - learning_rate * m_hat / (std::sqrt(v_hat) + eps);
      if (new_val < 0.01) // constrain params on the lower side
        new_val = 0.01;
      theta.set(ii, new_val);
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
GaussianProcessHandler::getLossAdam(RealEigenMatrix & inputs, RealEigenMatrix & outputs)
{
  _covariance_function->computeCovarianceMatrix(_K, inputs, inputs, true);
  setupStoredMatrices(outputs);
  Real log_likelihood = 0;
  log_likelihood += -(outputs.transpose() * _K_results_solve)(0, 0);
  log_likelihood += -std::log(_K.determinant());
  log_likelihood -= _batch_size * std::log(2 * M_PI);
  log_likelihood = -log_likelihood / 2;
  return log_likelihood;
}

std::vector<Real>
GaussianProcessHandler::getGradientAdam(RealEigenMatrix & inputs)
{
  RealEigenMatrix dKdhp(_batch_size, _batch_size);
  RealEigenMatrix alpha = _K_results_solve * _K_results_solve.transpose();
  std::vector<Real> grad_vec;
  grad_vec.resize(_num_tunable);
  int count;
  count = 1;
  for (auto iter = _tuning_data.begin(); iter != _tuning_data.end(); ++iter)
  {
    std::string hyper_param_name = iter->first;
    for (unsigned int ii = 0; ii < std::get<1>(iter->second); ++ii)
    {
      _covariance_function->computedKdhyper(dKdhp, inputs, hyper_param_name, ii);
      RealEigenMatrix tmp = alpha * dKdhp - _K_cho_decomp.solve(dKdhp);
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
  return grad_vec;
}

void
GaussianProcessHandler::mapToPetscVec(
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
GaussianProcessHandler::petscVecToMap(
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
dataStore(std::ostream & stream, StochasticTools::GaussianProcessHandler & gp_utils, void * context)
{
  dataStore(stream, gp_utils.hyperparamMap(), context);
  dataStore(stream, gp_utils.hyperparamVectorMap(), context);
  dataStore(stream, gp_utils.covarType(), context);
  dataStore(stream, gp_utils.K(), context);
  dataStore(stream, gp_utils.KResultsSolve(), context);
  dataStore(stream, gp_utils.KCholeskyDecomp(), context);
  dataStore(stream, gp_utils.paramStandardizer(), context);
  dataStore(stream, gp_utils.dataStandardizer(), context);
}

template <>
void
dataLoad(std::istream & stream, StochasticTools::GaussianProcessHandler & gp_utils, void * context)
{
  dataLoad(stream, gp_utils.hyperparamMap(), context);
  dataLoad(stream, gp_utils.hyperparamVectorMap(), context);
  dataLoad(stream, gp_utils.covarType(), context);
  dataLoad(stream, gp_utils.K(), context);
  dataLoad(stream, gp_utils.KResultsSolve(), context);
  dataLoad(stream, gp_utils.KCholeskyDecomp(), context);
  dataLoad(stream, gp_utils.paramStandardizer(), context);
  dataLoad(stream, gp_utils.dataStandardizer(), context);
}
