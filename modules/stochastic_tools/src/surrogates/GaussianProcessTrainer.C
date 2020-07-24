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
  params.addRequiredParam<SamplerName>("sampler", "Training set defined by a sampler object.");
  params.addRequiredParam<VectorPostprocessorName>(
      "results_vpp", "Vectorpostprocessor with results of samples created by trainer.");
  params.addRequiredParam<std::string>(
      "results_vector",
      "Name of vector from vectorpostprocessor with results of samples created by trainer");
  params.addRequiredParam<UserObjectName>("covariance_function", "Name of covariance function.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions", "Names of the distributions samples were taken from.");
  params.addParam<bool>(
      "standardize_params", true, "Standardize (center and scale) training parameters (x values)");
  params.addParam<bool>(
      "standardize_data", true, "Standardize (center and scale) training data (y values)");
  params.addParam<bool>("optimize", false, "Perform petsc optimize");
  return params;
}

GaussianProcessTrainer::GaussianProcessTrainer(const InputParameters & parameters)
  : SurrogateTrainer(parameters),
    CovarianceInterface(parameters),
    _training_params(declareModelData<RealEigenMatrix>("_training_params")),
    _param_standardizer(declareModelData<StochasticTools::Standardizer>("_param_standardizer")),
    _training_data(),
    _data_standardizer(declareModelData<StochasticTools::Standardizer>("_data_standardizer")),
    _K(declareModelData<RealEigenMatrix>("_K")),
    _K_results_solve(declareModelData<RealEigenMatrix>("_K_results_solve")),
    _standardize_params(getParam<bool>("standardize_params")),
    _standardize_data(getParam<bool>("standardize_data")),
    _covar_type(declareModelData<std::string>("_covar_type")),
    _hyperparam_map(declareModelData<std::unordered_map<std::string, Real>>("_hyperparam_map")),
    _hyperparam_vec_map(declareModelData<std::unordered_map<std::string, std::vector<Real>>>(
        "_hyperparam_vec_map")),
    _covariance_function(
        getCovarianceFunctionByName(getParam<UserObjectName>("covariance_function"))),
    _optimize(getParam<bool>("optimize"))

{
}

void
GaussianProcessTrainer::initialSetup()
{

  // Results VPP
  _values_distributed = isVectorPostprocessorDistributed("results_vpp");
  _values_ptr = &getVectorPostprocessorValue(
      "results_vpp", getParam<std::string>("results_vector"), !_values_distributed);

  // Sampler
  _sampler = &getSamplerByName(getParam<SamplerName>("sampler"));
  _n_params = _sampler->getNumberOfCols();

  // Check if sampler dimension matches number of distributions
  std::vector<DistributionName> dname = getParam<std::vector<DistributionName>>("distributions");
  if (dname.size() != _n_params)
    mooseError("Sampler number of columns does not match number of inputted distributions.");
}

void
GaussianProcessTrainer::initialize()
{
  // Check if results of samples matches number of samples
  __attribute__((unused)) dof_id_type num_rows =
      _values_distributed ? _sampler->getNumberOfLocalRows() : _sampler->getNumberOfRows();

  if (num_rows != _values_ptr->size())
    paramError("results_vpp",
               "The number of elements in '",
               getParam<VectorPostprocessorName>("results_vpp"),
               "/",
               getParam<std::string>("results_vector"),
               "' is not equal to the number of samples in '",
               getParam<SamplerName>("sampler"),
               "'!");

  _covar_type = _covariance_function->type();

  mooseAssert(_sampler->getNumberOfRows() == _values_ptr->size(),
              "Number of sampler rows not equal to number of results in selected VPP.");
}

void
GaussianProcessTrainer::execute()
{
  dof_id_type offset = _values_distributed ? _sampler->getLocalRowBegin() : 0;

  // Consider the possibility of a very large matrix load.
  _training_params.setZero(_sampler->getNumberOfRows(), _sampler->getNumberOfCols());
  _training_data.setZero(_sampler->getNumberOfRows(), 1);
  for (dof_id_type p = _sampler->getLocalRowBegin(); p < _sampler->getLocalRowEnd(); ++p)
  {
    // Loading parameters from sampler
    std::vector<Real> data = _sampler->getNextLocalRow();
    for (unsigned int d = 0; d < data.size(); ++d)
      _training_params(p, d) = data[d];

    // Loading result data from VPP
    _training_data(p, 0) = (*_values_ptr)[p - offset];
  }
}

void

GaussianProcessTrainer::finalize()
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

  if (_optimize)
    petscOptimize();

  _covariance_function->computeCovarianceMatrix(_K, _training_params, _training_params, true);
  _K_cho_decomp = _K.llt();
  _K_results_solve = _K_cho_decomp.solve(_training_data);

  _covariance_function->buildHyperParamMap(_hyperparam_map, _hyperparam_vec_map);
}

int
GaussianProcessTrainer::petscOptimize()
{
///////////////////////////////
///////////////////////////////
#ifdef LIBMESH_HAVE_PETSC
  std::cout << "have PETSC!" << '\n';
#endif // LIBMESH_HAVE_PETSC

  PetscErrorCode ierr;
  Vec theta, lower_vec, upper_vec;
  Tao tao;
  GaussianProcessTrainer * GP_ptr = this;
  PetscInt N;

  int num_hyper_params = _covariance_function->getNumTunable();
  VecCreate(PETSC_COMM_WORLD, &theta);
  VecSetSizes(theta, PETSC_DECIDE, num_hyper_params);
  VecSetFromOptions(theta);
  VecGetSize(theta, &N);
  VecSet(theta, 0);
  // VecView(theta,PETSC_VIEWER_STDOUT_WORLD);
  std::cout << N << "\n";

  libMesh::PetscVector<Number> TESTtheta(theta, _communicator);

  ierr = GaussianProcessTrainer::FormInitialGuess(GP_ptr, theta);
  std::cout << "Initial"
            << "\n";
  TESTtheta.print();
  // VecView(theta,PETSC_VIEWER_STDOUT_WORLD);

  // Get Bounds
  VecDuplicate(theta, &lower_vec);
  VecDuplicate(theta, &upper_vec);
  libMesh::PetscVector<Number> lower(lower_vec, _communicator);
  libMesh::PetscVector<Number> upper(upper_vec, _communicator);
  _covariance_function->buildHyperParamBounds(lower, upper);
  // VecView(lower_vec,PETSC_VIEWER_STDOUT_WORLD);
  // VecView(upper_vec,PETSC_VIEWER_STDOUT_WORLD);

  /* Create TAO solver and set desired solution method */
  ierr = TaoCreate(PETSC_COMM_WORLD, &tao);
  CHKERRQ(ierr);
  ierr = TaoSetType(tao, TAOBNCG);
  CHKERRQ(ierr);

  ierr = TaoSetVariableBounds(tao, lower_vec, upper_vec);
  CHKERRQ(ierr);

  // GRAD TESTING
  // Vec                grad;
  // VecCreate(PETSC_COMM_WORLD,&grad);
  // VecSetSizes(grad,PETSC_DECIDE,num_hyper_params);
  // VecSetFromOptions(grad);
  // VecSet(grad,0);
  // PetscReal fun=0;

  // GaussianProcessTrainer::FormFunctionGradientWrapper(tao, theta, &fun,grad, (void*)this);

  // END GRAD TESTING

  ierr = TaoSetInitialVector(tao, theta);
  CHKERRQ(ierr);
  ierr = TaoSetObjectiveAndGradientRoutine(
      tao, GaussianProcessTrainer::FormFunctionGradientWrapper, (void *)this);
  CHKERRQ(ierr);
  ierr = TaoSolve(tao);
  CHKERRQ(ierr);

  std::cout << "After solve"
            << "\n";
  TESTtheta.print();

  VecDestroy(&theta);
  // TODO add cleanup!

  return 0;
}

PetscErrorCode
GaussianProcessTrainer::FormInitialGuess(GaussianProcessTrainer * GP_ptr, Vec theta_vec)
{
  libMesh::PetscVector<Number> theta(theta_vec, GP_ptr->_communicator);
  GP_ptr->_covariance_function->buildHyperParamVec(theta);
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
GaussianProcessTrainer::FormFunctionGradient(Tao tao, Vec theta_vec, PetscReal * f, Vec grad_vec)
{
  libMesh::PetscVector<Number> theta(theta_vec, _communicator);
  libMesh::PetscVector<Number> grad(grad_vec, _communicator);

  _covariance_function->loadHyperParamVec(theta);
  _covariance_function->computeCovarianceMatrix(_K, _training_params, _training_params, true);
  _K_cho_decomp = _K.llt();
  _K_results_solve = _K_cho_decomp.solve(_training_data);

  // testing auto tuning
  int num_hyper_params = _covariance_function->getNumTunable();
  RealEigenMatrix dKdhp(_training_params.rows(), _training_params.rows());
  RealEigenMatrix alpha = _K_results_solve * _K_results_solve.transpose();
  for (int ii = 0; ii < num_hyper_params; ++ii)
  {
    _covariance_function->computedKdhyper(dKdhp, _training_params, ii);
    RealEigenMatrix tmp = alpha * dKdhp - _K_cho_decomp.solve(dKdhp);
    grad.set(ii, -tmp.trace() / 2.0);
  }
  //
  Real log_likelihood = 0;
  log_likelihood += -(_training_data.transpose() * _K_results_solve)(0, 0);
  log_likelihood += -std::log(_K.determinant());
  log_likelihood += -_training_data.rows() * std::log(2 * M_PI);
  log_likelihood = -log_likelihood / 2;
  std::cout << log_likelihood << '\n';
  *f = log_likelihood;
}
