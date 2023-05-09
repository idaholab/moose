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
  params.addClassDescription("Computes and evaluates Gaussian Process surrogate model.");
  return params;
}

GaussianProcess::GaussianProcess(const InputParameters & parameters)
  : SurrogateModel(parameters),
    CovarianceInterface(parameters),
    _gp_handler(declareModelData<StochasticTools::GaussianProcessHandler>("_gp_handler")),
    _training_params(getModelData<RealEigenMatrix>("_training_params"))
{
}

void
GaussianProcess::setupCovariance(UserObjectName covar_name)
{
  if (_gp_handler.getCovarFunctionPtr() != nullptr)
    ::mooseError("Attempting to redefine covariance function using setupCovariance.");
  _gp_handler.linkCovarianceFunction(getCovarianceFunctionByName(covar_name));
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

  unsigned int _n_params = _training_params.cols();
  unsigned int _num_tests = 1;

  mooseAssert(x.size() == _n_params,
              "Number of parameters provided for evaluation does not match number of parameters "
              "used for training.");

  RealEigenMatrix test_points(_num_tests, _n_params);
  for (unsigned int ii = 0; ii < _n_params; ++ii)
    test_points(0, ii) = x[ii];

  _gp_handler.getParamStandardizer().getStandardized(test_points);

  RealEigenMatrix K_train_test(_training_params.rows(), test_points.rows());
  _gp_handler.getCovarFunction().computeCovarianceMatrix(
      K_train_test, _training_params, test_points, false);
  RealEigenMatrix K_test(test_points.rows(), test_points.rows());
  _gp_handler.getCovarFunction().computeCovarianceMatrix(K_test, test_points, test_points, true);

  // Compute the predicted mean value (centered)
  RealEigenMatrix pred_value = (K_train_test.transpose() * _gp_handler.getKResultsSolve());
  // De-center/scale the value and store for return
  _gp_handler.getDataStandardizer().getDestandardized(pred_value);

  RealEigenMatrix pred_var =
      K_test - (K_train_test.transpose() * _gp_handler.getKCholeskyDecomp().solve(K_train_test));

  // Vairance computed, take sqrt for standard deviation, scale up by training data std and store
  RealEigenMatrix std_dev_mat = pred_var.array().sqrt();
  _gp_handler.getDataStandardizer().getDescaled(std_dev_mat);
  std_dev = std_dev_mat(0, 0);

  return pred_value(0, 0);
}
