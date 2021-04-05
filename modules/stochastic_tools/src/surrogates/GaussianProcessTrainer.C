//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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

#include <math.h>

registerMooseObject("StochasticToolsApp", GaussianProcessTrainer);

InputParameters
GaussianProcessTrainer::validParams()
{
  InputParameters params = SurrogateTrainer::validParams();
  params += CovarianceInterface::validParams();
  params.addClassDescription(
      "Provides data preperation and training for a Gaussian Process surrogate model.");
  params.addRequiredParam<ReporterName>(
      "response", "Reporter value of response results, can be vpp with <vpp_name>/<vector_name>.");
  params.addParam<std::vector<ReporterName>>(
      "predictors",
      std::vector<ReporterName>(),
      "Reporter values used as the independent random variables, If 'predictors' and "
      "'predictor_cols' are both empty, all sampler columns are used.");
  params.addParam<std::vector<unsigned int>>(
      "predictor_cols",
      std::vector<unsigned int>(),
      "Sampler columns used as the independent random variables, If 'predictors' and "
      "'predictor_cols' are both empty, all sampler columns are used.");
  params.addRequiredParam<UserObjectName>("covariance_function", "Name of covariance function.");
  params.addParam<bool>(
      "standardize_params", true, "Standardize (center and scale) training parameters (x values)");
  params.addParam<bool>(
      "standardize_data", true, "Standardize (center and scale) training data (y values)");
#ifdef LIBMESH_HAVE_PETSC
  params.addParam<std::string>(
      "tao_options", "", "Command line options for PETSc/TAO hyperparameter optimization");
  params.addParam<bool>("show_tao", false, "Switch to show TAO solver results");
  params.addParam<std::vector<std::string>>("tune_parameters",
                                            "Select hyperparameters to be tuned");
  params.addParam<std::vector<Real>>("tuning_min", "Minimum allowable tuning value");
  params.addParam<std::vector<Real>>("tuning_max", "Maximum allowable tuning value");
#endif
  return params;
}

GaussianProcessTrainer::GaussianProcessTrainer(const InputParameters & parameters)
  : SurrogateTrainer(parameters),
    CovarianceInterface(parameters),
    _training_params(declareModelData<RealEigenMatrix>("_training_params")),
    _param_standardizer(declareModelData<StochasticTools::Standardizer>("_param_standardizer")),
    _data_standardizer(declareModelData<StochasticTools::Standardizer>("_data_standardizer")),
    _K(declareModelData<RealEigenMatrix>("_K")),
    _K_results_solve(declareModelData<RealEigenMatrix>("_K_results_solve")),
    _K_cho_decomp(declareModelData<Eigen::LLT<RealEigenMatrix>>("_K_cho_decomp")),
    _standardize_params(getParam<bool>("standardize_params")),
    _standardize_data(getParam<bool>("standardize_data")),
    _covariance_function(
        getCovarianceFunctionByName(getParam<UserObjectName>("covariance_function"))),
    _covar_type(declareModelData<std::string>("_covar_type", _covariance_function->type())),
#ifdef LIBMESH_HAVE_PETSC
    _do_tuning(isParamValid("tune_parameters")),
    _tao_options(getParam<std::string>("tao_options")),
    _show_tao(getParam<bool>("show_tao")),
    _tao_comm(MPI_COMM_SELF),
#endif
    _hyperparam_map(declareModelData<std::unordered_map<std::string, Real>>("_hyperparam_map")),
    _hyperparam_vec_map(declareModelData<std::unordered_map<std::string, std::vector<Real>>>(
        "_hyperparam_vec_map")),
    _sampler_row(getSamplerData()),
    _rval(getTrainingData<Real>(getParam<ReporterName>("response"))),
    _pvals(getParam<std::vector<ReporterName>>("predictors").size()),
    _pcols(getParam<std::vector<unsigned int>>("predictor_cols")),
    _n_params((_pvals.empty() && _pcols.empty()) ? _sampler.getNumberOfCols()
                                                 : (_pvals.size() + _pcols.size()))
{
  const auto & pnames = getParam<std::vector<ReporterName>>("predictors");
  for (unsigned int i = 0; i < pnames.size(); ++i)
    _pvals[i] = &getTrainingData<Real>(pnames[i]);

  // If predictors and predictor_cols are empty, use all sampler columns
  if (_pvals.empty() && _pcols.empty())
  {
    _pcols.resize(_sampler.getNumberOfCols());
    std::iota(_pcols.begin(), _pcols.end(), 0);
  }

#ifdef LIBMESH_HAVE_PETSC
  _num_tunable = 0;
  std::vector<std::string> tune_parameters(getParam<std::vector<std::string>>("tune_parameters"));
  // Error Checking
  if (isParamValid("tuning_min") &&
      (getParam<std::vector<Real>>("tuning_min").size() != tune_parameters.size()))
    mooseError("tuning_min size does not match tune_parameters");
  if (isParamValid("tuning_max") &&
      (getParam<std::vector<Real>>("tuning_max").size() != tune_parameters.size()))
    mooseError("tuning_max size does not match tune_parameters");
  // Fill Out Tunable Paramater information
  for (unsigned int ii = 0; ii < tune_parameters.size(); ++ii)
  {
    const auto & hp = tune_parameters[ii];
    if (_covariance_function->isTunable(hp))
    {
      unsigned int size;
      Real min;
      Real max;
      // Get size and default min/max
      _covariance_function->getTuningData(hp, size, min, max);
      // Check for overridden min/max
      min = isParamValid("tuning_min") ? getParam<std::vector<Real>>("tuning_min")[ii] : min;
      max = isParamValid("tuning_max") ? getParam<std::vector<Real>>("tuning_max")[ii] : max;
      // Save data in tuple
      _tuning_data[hp] = std::make_tuple(_num_tunable, size, min, max);
      _num_tunable += size;
    }
  }
#endif
}

void
GaussianProcessTrainer::preTrain()
{
  _training_params.setZero(_sampler.getNumberOfRows(), _n_params);
  _training_data.setZero(_sampler.getNumberOfRows(), 1);
}

void
GaussianProcessTrainer::train()
{
  unsigned int d = 0;
  for (const auto & val : _pvals)
    _training_params(_row, d++) = *val;
  for (const auto & col : _pcols)
    _training_params(_row, d++) = _sampler_row[col];

  // Loading result data from response reporter
  _training_data(_row, 0) = _rval;
}

void
GaussianProcessTrainer::postTrain()
{
  for (unsigned int ii = 0; ii < _training_params.rows(); ++ii)
  {
    for (unsigned int jj = 0; jj < _training_params.cols(); ++jj)
      gatherSum(_training_params(ii, jj));
    gatherSum(_training_data(ii, 0));
  }

  // Standardize (center and scale) training params
  if (_standardize_params)
  {
    _param_standardizer.computeSet(_training_params);
    _param_standardizer.getStandardized(_training_params);
  }
  // if not standardizing data set mean=0, std=1 for use in surrogate
  else
    _param_standardizer.set(0, 1, _n_params);

  // Standardize (center and scale) training data
  if (_standardize_data)
  {
    _data_standardizer.computeSet(_training_data);
    _data_standardizer.getStandardized(_training_data);
  }
  // if not standardizing data set mean=0, std=1 for use in surrogate
  else
    _param_standardizer.set(0, 1, _n_params);

  _K.resize(_training_params.rows(), _training_params.rows());

#ifdef LIBMESH_HAVE_PETSC
  if (_do_tuning)
    if (hyperparamTuning())
      mooseError("PETSc/TAO error in hyperparameter tuning.");
#endif

  _covariance_function->computeCovarianceMatrix(_K, _training_params, _training_params, true);
  _K_cho_decomp = _K.llt();
  _K_results_solve = _K_cho_decomp.solve(_training_data);

  _covariance_function->buildHyperParamMap(_hyperparam_map, _hyperparam_vec_map);
}

#ifdef LIBMESH_HAVE_PETSC
PetscErrorCode
GaussianProcessTrainer::hyperparamTuning()
{
  PetscErrorCode ierr;
  Tao tao;
  GaussianProcessTrainer * GP_ptr = this;

  // Setup Tao optimization problem
  ierr = TaoCreate(_tao_comm.get(), &tao);
  CHKERRQ(ierr);
  ierr = PetscOptionsSetValue(NULL, "-tao_type", "bncg");
  CHKERRQ(ierr);
  ierr = PetscOptionsInsertString(NULL, _tao_options.c_str());
  CHKERRQ(ierr);
  ierr = TaoSetFromOptions(tao);
  CHKERRQ(ierr);

  // Define petsc vetor to hold tunalbe hyper-params
  libMesh::PetscVector<Number> theta(_tao_comm, _num_tunable);
  ierr = GaussianProcessTrainer::FormInitialGuess(GP_ptr, theta.vec());
  CHKERRQ(ierr);
  ierr = TaoSetInitialVector(tao, theta.vec());
  CHKERRQ(ierr);

  // Get Hyperparameter bounds.
  libMesh::PetscVector<Number> lower(_tao_comm, _num_tunable);
  libMesh::PetscVector<Number> upper(_tao_comm, _num_tunable);
  buildHyperParamBounds(lower, upper);
  CHKERRQ(ierr);
  ierr = TaoSetVariableBounds(tao, lower.vec(), upper.vec());
  CHKERRQ(ierr);

  // Set Objective and Graident Callback
  ierr = TaoSetObjectiveAndGradientRoutine(
      tao, GaussianProcessTrainer::FormFunctionGradientWrapper, (void *)this);
  CHKERRQ(ierr);

  // Solve
  ierr = TaoSolve(tao);
  CHKERRQ(ierr);
  //
  if (_show_tao)
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
GaussianProcessTrainer::FormInitialGuess(GaussianProcessTrainer * GP_ptr, Vec theta_vec)
{
  libMesh::PetscVector<Number> theta(theta_vec, GP_ptr->_tao_comm);
  _covariance_function->buildHyperParamMap(_hyperparam_map, _hyperparam_vec_map);
  mapToVec(theta);
  return 0;
}

PetscErrorCode
GaussianProcessTrainer::FormFunctionGradientWrapper(
    Tao tao, Vec theta_vec, PetscReal * f, Vec grad_vec, void * ptr)
{
  GaussianProcessTrainer * GP_ptr = (GaussianProcessTrainer *)ptr;
  GP_ptr->FormFunctionGradient(tao, theta_vec, f, grad_vec);
  return 0;
}

void
GaussianProcessTrainer::FormFunctionGradient(Tao /*tao*/,
                                             Vec theta_vec,
                                             PetscReal * f,
                                             Vec grad_vec)
{
  libMesh::PetscVector<Number> theta(theta_vec, _tao_comm);
  libMesh::PetscVector<Number> grad(grad_vec, _tao_comm);

  vecToMap(theta);
  _covariance_function->loadHyperParamMap(_hyperparam_map, _hyperparam_vec_map);
  _covariance_function->computeCovarianceMatrix(_K, _training_params, _training_params, true);
  _K_cho_decomp = _K.llt();
  _K_results_solve = _K_cho_decomp.solve(_training_data);

  // testing auto tuning
  RealEigenMatrix dKdhp(_training_params.rows(), _training_params.rows());
  RealEigenMatrix alpha = _K_results_solve * _K_results_solve.transpose();
  for (auto iter = _tuning_data.begin(); iter != _tuning_data.end(); ++iter)
  {
    std::string hyper_param_name = iter->first;
    for (unsigned int ii = 0; ii < std::get<1>(iter->second); ++ii)
    {
      _covariance_function->computedKdhyper(dKdhp, _training_params, hyper_param_name, ii);
      RealEigenMatrix tmp = alpha * dKdhp - _K_cho_decomp.solve(dKdhp);
      grad.set(std::get<0>(iter->second) + ii, -tmp.trace() / 2.0);
    }
  }
  //
  Real log_likelihood = 0;
  log_likelihood += -(_training_data.transpose() * _K_results_solve)(0, 0);
  log_likelihood += -std::log(_K.determinant());
  log_likelihood += -_training_data.rows() * std::log(2 * M_PI);
  log_likelihood = -log_likelihood / 2;
  *f = log_likelihood;
}

void
GaussianProcessTrainer::buildHyperParamBounds(libMesh::PetscVector<Number> & theta_l,
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

void
GaussianProcessTrainer::mapToVec(libMesh::PetscVector<Number> & theta) const
{
  for (auto iter = _tuning_data.begin(); iter != _tuning_data.end(); ++iter)
  {
    std::string hyper_param_name = iter->first;
    if (_hyperparam_map.find(hyper_param_name) != _hyperparam_map.end())
    {
      theta.set(std::get<0>(iter->second), _hyperparam_map[hyper_param_name]);
    }
    else if (_hyperparam_vec_map.find(hyper_param_name) != _hyperparam_vec_map.end())
    {
      for (unsigned int ii = 0; ii < std::get<1>(iter->second); ++ii)
      {
        theta.set(std::get<0>(iter->second) + ii, _hyperparam_vec_map[hyper_param_name][ii]);
      }
    }
  }
}

void
GaussianProcessTrainer::vecToMap(libMesh::PetscVector<Number> & theta)
{
  for (auto iter = _tuning_data.begin(); iter != _tuning_data.end(); ++iter)
  {
    std::string hyper_param_name = iter->first;
    if (_hyperparam_map.find(hyper_param_name) != _hyperparam_map.end())
    {
      _hyperparam_map[hyper_param_name] = theta(std::get<0>(iter->second));
    }
    else if (_hyperparam_vec_map.find(hyper_param_name) != _hyperparam_vec_map.end())
    {
      for (unsigned int ii = 0; ii < std::get<1>(iter->second); ++ii)
      {
        _hyperparam_vec_map[hyper_param_name][ii] = theta(std::get<0>(iter->second) + ii);
      }
    }
  }
}
#endif // LIBMESH_HAVE_PETSC

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
