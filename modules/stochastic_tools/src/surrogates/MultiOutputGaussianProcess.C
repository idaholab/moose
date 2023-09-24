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
  // std::cout << "Test: " << Moose::stringify(test_points) << std::endl;
  RealEigenMatrix K_train = _mogp_handler.getK();
  RealEigenMatrix K_train_test(test_points.rows(), K_train.rows());
  // RealEigenMatrix batch_inp = _mogp_handler.getBatchInputs();
  _mogp_handler.getCovarFunction().computeCovarianceMatrix(
      K_train_test, test_points, _training_params, false); // batch_inp

  // std::cout << "Test: " << Moose::stringify(_training_params.row(0)) << std::endl;
  // RealEigenMatrix K_train = _mogp_handler.getK();
  // RealEigenMatrix K_train_test(test_points.rows(), K_train.rows());
  // RealEigenMatrix batch_inp = _mogp_handler.getBatchInputs();
  // _mogp_handler.getCovarFunction().computeCovarianceMatrix(
  //     K_train_test, _training_params.row(0), batch_inp, false);

  // Outputs
  RealEigenMatrix B = _mogp_handler.getB();
  RealEigenMatrix kappa_results_solve = _mogp_handler.getKappaResultsSolve();
  // std::cout << Moose::stringify(kappa_results_solve) << std::endl;
  // std::cout << kappa_results_solve.rows() << " cols " << kappa_results_solve.cols() << std::endl;
  RealEigenMatrix kappa_train_test;
  // K_train_test = K_train_test.transpose();
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

  // for (unsigned int i = 0; i < kappa_train_test_new.cols(); ++i)
  // {
  //   std::cout << kappa_train_test_new(1, i) << "," << std::endl;
  // }
  // RealEigenMatrix mogp_mean = kappa_train_test * kappa_results_solve;
  RealEigenMatrix mogp_mean = kappa_train_test_new * kappa_results_solve;
  // std::cout << Moose::stringify(mogp_mean) << std::endl;
  _mogp_handler.getDataStandardizer().getDestandardizedCovariance(mogp_mean); //

  // _mogp_handler.getOutputCovar().computeFullCovarianceMatrix(kappa_train_test, K_train_test.transpose(), B);
  // std::cout << Moose::stringify(B) << std::endl;

  std::cout << Moose::stringify(mogp_mean) << std::endl;

  // RealEigenMatrix test(2,1);
  // test(0, 0) = 836.5923827409;
  // test(1, 0) = 517.89436917846;
  // _mogp_handler.getDataStandardizer().getStandardizedCovariance(test);
  // std::cout << "True " << Moose::stringify(test) << std::endl;

  // RealEigenMatrix kappa_test;
  // _mogp_handler.getOutputCovar().computeFullCovarianceMatrix(kappa_test, B, K_test);

  y.resize(mogp_mean.rows());
  for (unsigned int i = 0; i < mogp_mean.rows(); ++i)
    y[i] = mogp_mean(i, 0);
}
