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

registerMooseObject("StochasticToolsApp", GaussianProcess);

InputParameters
GaussianProcess::validParams()
{
  InputParameters params = SurrogateModel::validParams();
  params.addClassDescription("Computes and evaluates Gaussian Process surrogate model.");
  return params;
}

GaussianProcess::GaussianProcess(const InputParameters & parameters)
  : SurrogateModel(parameters),
    _sample_points(getModelData<std::vector<std::vector<Real>>>("_sample_points")),
    _training_params(getModelData<RealEigenMatrix>("_training_params")),
    _training_params_mean(getModelData<RealEigenVector>("_training_params_mean")),
    _training_params_var(getModelData<RealEigenVector>("_training_params_var")),
    _training_data(getModelData<RealEigenMatrix>("_training_data")),
    _training_data_mean(getModelData<RealEigenVector>("_training_data_mean")),
    _training_data_var(getModelData<RealEigenVector>("_training_data_var")),
    _K(getModelData<RealEigenMatrix>("_K")),
    _K_results_solve(getModelData<RealEigenMatrix>("_K_results_solve")),
    _covar_function(
        getModelData<std::unique_ptr<CovarianceFunction::CovarianceKernel>>("_covar_function"))
{
}

Real
GaussianProcess::evaluate(const std::vector<Real> & x) const
{
  // overlaod for evaluate to maintain general compatibility. Only returns mean
  Real dummy = 0;
  return this->evaluate(x, dummy);
}

Real
GaussianProcess::evaluate(const std::vector<Real> & x, Real & std_dev) const
{
  unsigned int _n_params = _training_params.cols();
  unsigned int _num_tests = 1;

  // assert x.size() == _n_params

  RealEigenMatrix test_points(_num_tests, _n_params);
  for (unsigned int ii = 0; ii < _n_params; ii++)
  {
    test_points(0, ii) =
        (x.at(ii) - _training_params_mean(ii)) / std::sqrt(_training_params_var(ii));
  }

  RealEigenMatrix K_train_test = _covar_function->compute_K(_training_params, test_points, false);
  RealEigenMatrix K_test = _covar_function->compute_K(test_points, test_points, true);

  // Compute the predicted mean value (centered)
  Real mean_value = (K_train_test.transpose() * _K_results_solve)(0, 0);
  // De-center/scale the value and store for return
  mean_value = (mean_value * _training_data_var.array().sqrt()(0)) + _training_data_mean(0);

  // Compute standard deviation
  Eigen::LLT<RealEigenMatrix> _K_cho_decomp(_K);
  RealEigenMatrix test_std =
      K_test - (K_train_test.transpose() * _K_cho_decomp.solve(K_train_test));

  // Vairance computed, take sqrt for standard deviation, scale up by training data std and store
  std_dev = std::sqrt(test_std(0, 0)) * _training_data_var.array().sqrt()(0);

  return mean_value;
}
