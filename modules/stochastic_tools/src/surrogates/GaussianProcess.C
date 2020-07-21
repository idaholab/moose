//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GaussianProcess.h"
#include "Sampler.h"

#include "CovarianceFunctionBase.h"

registerMooseObject("StochasticToolsApp", GaussianProcess);

InputParameters
GaussianProcess::validParams()
{
  InputParameters params = SurrogateModel::validParams();
  params += CovarianceInterface::validParams();
  params.addClassDescription("Computes and evaluates Gaussian Process surrogate model.");
  return params;
}

GaussianProcess::GaussianProcess(const InputParameters & parameters)
  : SurrogateModel(parameters),
    CovarianceInterface(parameters),
    _training_params(getModelData<RealEigenMatrix>("_training_params")),
    _param_standardizer(getModelData<StochasticTools::Standardizer>("_param_standardizer")),
    _data_standardizer(getModelData<StochasticTools::Standardizer>("_data_standardizer")),
    _K(getModelData<RealEigenMatrix>("_K")),
    _K_results_solve(getModelData<RealEigenMatrix>("_K_results_solve")),
    _covar_type(getModelData<std::string>("_covar_type")),
    _hyperparam_map(getModelData<std::unordered_map<std::string, Real>>("_hyperparam_map")),
    _hyperparam_vec_map(
        getModelData<std::unordered_map<std::string, std::vector<Real>>>("_hyperparam_vec_map")),
    _covariance_function(
        isParamValid("trainer")
            ? dynamic_cast<GaussianProcessTrainer &>(getSurrogateTrainer("trainer")).getCovarPtr()
            : nullptr)
{
}

void
GaussianProcess::setupCovariance(UserObjectName covar_name)
{
  if (_covariance_function != nullptr)
    ::mooseError("Attempting to redefine covariance function using setupCovariance.");
  _covariance_function = getCovarianceFunctionByName(covar_name);
}

Real
GaussianProcess::evaluate(const std::vector<Real> & x) const
{
  // Overlaod for evaluate to maintain general compatibility. Only returns mean
  Real dummy = 0;
  return this->evaluate(x, dummy);
}

Real
GaussianProcess::evaluate(const std::vector<Real> & x, Real & std_dev) const
{

  // TODO: This decomposition should be in an initalize() function, it only needs to be called once.
  // Issue #15482
  Eigen::LLT<RealEigenMatrix> _K_cho_decomp(_K);

  unsigned int _n_params = _training_params.cols();
  unsigned int _num_tests = 1;

  mooseAssert(x.size() == _n_params,
              "Number of parameters provided for evaluation does not match number of parameters "
              "used for training.");

  RealEigenMatrix test_points(_num_tests, _n_params);
  for (unsigned int ii = 0; ii < _n_params; ++ii)
    test_points(0, ii) = x[ii];

  _param_standardizer.getStandardized(test_points);

  RealEigenMatrix K_train_test(_training_params.rows(), test_points.rows());
  _covariance_function->computeCovarianceMatrix(K_train_test, _training_params, test_points, false);
  RealEigenMatrix K_test(test_points.rows(), test_points.rows());
  _covariance_function->computeCovarianceMatrix(K_test, test_points, test_points, true);

  // Compute the predicted mean value (centered)
  RealEigenMatrix pred_value = (K_train_test.transpose() * _K_results_solve);
  // De-center/scale the value and store for return
  _data_standardizer.getDestandardized(pred_value);

  RealEigenMatrix pred_var =
      K_test - (K_train_test.transpose() * _K_cho_decomp.solve(K_train_test));

  // Vairance computed, take sqrt for standard deviation, scale up by training data std and store
  RealEigenMatrix std_dev_mat = pred_var.array().sqrt();
  _data_standardizer.getDescaled(std_dev_mat);
  std_dev = std_dev_mat(0, 0);

  return pred_value(0, 0);
}
