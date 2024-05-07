//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiOutputGaussianProcessSurrogate.h"
#include "Sampler.h"

#include "CovarianceFunctionBase.h"

registerMooseObject("StochasticToolsApp", MultiOutputGaussianProcessSurrogate);

InputParameters
MultiOutputGaussianProcessSurrogate::validParams()
{
  InputParameters params = SurrogateModel::validParams();
  params.addClassDescription(
      "Computes and evaluates Multi-Output Gaussian Process surrogate model.");
  return params;
}

MultiOutputGaussianProcessSurrogate::MultiOutputGaussianProcessSurrogate(
    const InputParameters & parameters)
  : SurrogateModel(parameters),
    CovarianceInterface(parameters),
    OutputCovarianceInterface(parameters),
    _mogp_handler(declareModelData<StochasticTools::MultiOutputGaussianProcess>("_mogp_handler")),
    _training_params(getModelData<RealEigenMatrix>("_training_params")),
    _kappa_cho_decomp(_mogp_handler.getKappaCholeskyDecomp()),
    _K_train(_mogp_handler.getK()),
    _B(_mogp_handler.getB()),
    _kappa_results_solve(_mogp_handler.getKappaResultsSolve())
{
}

void
MultiOutputGaussianProcessSurrogate::evaluate(const std::vector<Real> & x,
                                              std::vector<Real> & y) const
{
  // Gather the required inputs and matrices
  unsigned int n_params = _training_params.cols();
  mooseAssert(x.size() == n_params,
              "Number of parameters provided for evaluation does not match number of parameters "
              "used for training.");

  // Gather the test points and standardize
  RealEigenMatrix test_points(1, n_params);
  for (unsigned int ii = 0; ii < n_params; ++ii)
    test_points(0, ii) = x[ii];
  _mogp_handler.getParamStandardizer().getStandardized(test_points);

  // Compute full test-train covariance and arrange it properly
  RealEigenMatrix K_train_test(test_points.rows(), _K_train.rows());
  _mogp_handler.getCovarFunction().computeCovarianceMatrix(
      K_train_test, test_points, _training_params, false);
  RealEigenMatrix kappa_train_test;
  _mogp_handler.getOutputCovar().computeFullCovarianceMatrix(kappa_train_test, K_train_test, _B);
  RealEigenMatrix kappa_train_test_new(kappa_train_test.rows(), kappa_train_test.cols());
  RealEigenMatrix tmp;
  for (unsigned int j = 0; j < kappa_train_test.rows(); ++j)
  {
    unsigned int count = 0;
    for (unsigned int i = 0; i < kappa_train_test.rows(); ++i)
    {
      tmp = kappa_train_test(j, Eigen::seq(i, kappa_train_test.cols() - 1, _B.rows()));
      kappa_train_test_new(j, Eigen::seq(i * count, i * count + tmp.cols() - 1)) = tmp;
      count += tmp.cols();
    }
  }

  // Compute full test covariance
  RealEigenMatrix K_test(test_points.rows(), test_points.rows());
  _mogp_handler.getCovarFunction().computeCovarianceMatrix(K_test, test_points, test_points, true);
  RealEigenMatrix kappa_test;
  _mogp_handler.getOutputCovar().computeFullCovarianceMatrix(kappa_test, K_test, _B);
  RealEigenMatrix kappa_test_solve = _kappa_cho_decomp.solve(kappa_train_test_new.transpose());
  RealEigenMatrix covar_test = kappa_test - kappa_train_test_new * kappa_test_solve;
  RealEigenVector mogp_var(covar_test.rows());
  for (unsigned int i = 0; i < mogp_var.rows(); ++i)
    mogp_var(i) = (covar_test(i, i)); // std::sqrt
  _mogp_handler.getDataStandardizer().getDescaledCovariance(mogp_var);

  // Compute test mean predictions
  RealEigenMatrix mogp_mean = kappa_train_test_new * _kappa_results_solve;

  // Broadcast normalized mean and std to SurrogateModel
  y.assign(mogp_mean.rows() * 4, 0.0);
  for (unsigned int i = 0; i < mogp_mean.rows(); ++i)
    y[i] = mogp_mean(i, 0);
  for (unsigned int i = mogp_mean.rows(); i < 2 * mogp_mean.rows(); ++i)
    y[i] = std::sqrt(covar_test(i - mogp_mean.rows(), i - mogp_mean.rows()));

  // Broadcast unnormalized mean and std to SurrogateModel
  _mogp_handler.getDataStandardizer().getDestandardizedCovariance(mogp_mean);
  for (unsigned int i = 2 * mogp_mean.rows(); i < 3 * mogp_mean.rows(); ++i)
    y[i] = mogp_mean(i - 2 * mogp_mean.rows(), 0);
  for (unsigned int i = 3 * mogp_mean.rows(); i < 4 * mogp_mean.rows(); ++i)
    y[i] = std::sqrt(mogp_var(i - 3 * mogp_mean.rows()));
}
