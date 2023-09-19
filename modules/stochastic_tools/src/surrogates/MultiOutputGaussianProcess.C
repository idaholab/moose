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
  std::cout << "Test: " << Moose::stringify(test_points) << std::endl;
  _mogp_handler.getParamStandardizer().getStandardized(test_points);
  RealEigenMatrix K_train_test(_training_params.rows(), test_points.rows());
  _mogp_handler.getCovarFunction().computeCovarianceMatrix(
      K_train_test, _training_params, test_points, false);
  K_train_test = K_train_test.transpose();
  // RealEigenMatrix K_test(test_points.rows(), test_points.rows());
  // _mogp_handler.getCovarFunction().computeCovarianceMatrix(K_test, test_points, test_points, true);
  RealEigenMatrix K_train = _mogp_handler.getK();

  // Outputs
  RealEigenMatrix B = _mogp_handler.getB();
  RealEigenMatrix kappa_results_solve = _mogp_handler.getKappaResultsSolve();
  RealEigenMatrix kappa_train_test;
  _mogp_handler.getOutputCovar().computeFullCovarianceMatrix(kappa_train_test, B, K_train_test);
  RealEigenMatrix mogp_mean = kappa_train_test * kappa_results_solve;
  mogp_mean = mogp_mean.transpose();
  std::cout << "mogp_mean " << Moose::stringify(mogp_mean) << std::endl;
  _mogp_handler.getDataStandardizer().getDestandardized(mogp_mean); // Covariance
  std::cout << Moose::stringify(mogp_mean) << std::endl;

  y.resize(mogp_mean.rows());
  for (unsigned int i = 0; i < mogp_mean.rows(); ++i)
    y[i] = mogp_mean(i, 0);

  // std::cout << Moose::stringify(mogp_mean) << std::endl;
  // std::cout << mogp_mean.rows() << " cols " << mogp_mean.cols() << std::endl;
  // std::cout << "Finished ********" << std::endl;

  // // Compute the predicted mean value (centered)
  // RealEigenMatrix pred_value = (K_train_test.transpose() * _mogp_handler.getKResultsSolve());
  // // De-center/scale the value and store for return
  // _mogp_handler.getDataStandardizer().getDestandardized(pred_value);

  // RealEigenMatrix pred_var =
  //     K_test - (K_train_test.transpose() * _mogp_handler.getKCholeskyDecomp().solve(K_train_test));

  // // Vairance computed, take sqrt for standard deviation, scale up by training data std and store
  // RealEigenMatrix std_dev_mat = pred_var.array().sqrt();
  // _mogp_handler.getDataStandardizer().getDescaled(std_dev_mat);
  // std_dev = std_dev_mat(0, 0);
}
