//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiOutputGaussianProcess.h"
#include "Sampler.h"

#include "CovarianceFunctionBase.h"

registerMooseObject("StochasticToolsApp", MultiOutputGaussianProcess);

InputParameters
MultiOutputGaussianProcess::validParams()
{
  InputParameters params = SurrogateModel::validParams();
  params.addClassDescription("Computes and evaluates Gaussian Process surrogate model.");
  return params;
}

MultiOutputGaussianProcess::MultiOutputGaussianProcess(const InputParameters & parameters)
  : SurrogateModel(parameters),
    CovarianceInterface(parameters),
    OutputCovarianceInterface(parameters),
    _mogp_handler(
        declareModelData<StochasticTools::MultiOutputGaussianProcessHandler>("_mogp_handler")),
    _training_params(getModelData<RealEigenMatrix>("_training_params"))
{
}

// void
// MultiOutputGaussianProcess::setupCovariance(UserObjectName covar_name)
// {
//   if (_mogp_handler.getCovarFunctionPtr() != nullptr)
//     ::mooseError("Attempting to redefine covariance function using setupCovariance.");
//   _mogp_handler.linkCovarianceFunction(getCovarianceFunctionByName(covar_name));
// }

void
MultiOutputGaussianProcess::evaluate(const std::vector<Real> & x, std::vector<Real> & y) const
{

  unsigned int _n_params = _training_params.cols();
  unsigned int _num_tests = 1;

  mooseAssert(x.size() == _n_params,
              "Number of parameters provided for evaluation does not match number of parameters "
              "used for training.");

  RealEigenMatrix test_points(_num_tests, _n_params);
  for (unsigned int ii = 0; ii < _n_params; ++ii)
    test_points(0, ii) = x[ii];

  // Inputs
  _mogp_handler.getParamStandardizer().getStandardized(test_points);
  RealEigenMatrix K_train = _mogp_handler.getK();
  RealEigenMatrix K_train_test(test_points.rows(), K_train.rows());
  RealEigenMatrix K_test(test_points.rows(), test_points.rows());
  _mogp_handler.getCovarFunction().computeCovarianceMatrix(
      K_train_test, test_points, _training_params, false);
  _mogp_handler.getCovarFunction().computeCovarianceMatrix(K_test, test_points, test_points, true);

  // Outputs
  RealEigenMatrix B = _mogp_handler.getB();
  RealEigenMatrix kappa_results_solve = _mogp_handler.getKappaResultsSolve();
  RealEigenMatrix kappa_train_test;
  _mogp_handler.getOutputCovar().computeFullCovarianceMatrix(kappa_train_test, K_train_test, B);

  // KEY PIECE OF THE CODE
  RealEigenMatrix kappa_train_test_new(kappa_train_test.rows(), kappa_train_test.cols());
  RealEigenMatrix tmp;
  for (unsigned int j = 0; j < kappa_train_test.rows(); ++j) //
  {
    unsigned int count = 0;
    for (unsigned int i = 0; i < kappa_train_test.rows(); ++i)
    {
      tmp = kappa_train_test(j, Eigen::seq(i, kappa_train_test.cols() - 1, B.rows()));
      kappa_train_test_new(j, Eigen::seq(i * count, i * count + tmp.cols() - 1)) = tmp;
      count += tmp.cols();
    }
  }

  RealEigenMatrix mogp_mean = kappa_train_test_new * kappa_results_solve;
  _mogp_handler.getDataStandardizer().getDestandardizedCovariance(mogp_mean);

  y.assign(mogp_mean.rows(), 0.0);
  for (unsigned int i = 0; i < mogp_mean.rows(); ++i)
    y[i] = mogp_mean(i, 0);
}
