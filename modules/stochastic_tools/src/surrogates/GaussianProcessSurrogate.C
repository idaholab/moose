//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#ifdef LIBTORCH_ENABLED

#include "GaussianProcessSurrogate.h"
#include "Sampler.h"

#include "CovarianceFunctionBase.h"

registerMooseObject("StochasticToolsApp", GaussianProcessSurrogate);

InputParameters
GaussianProcessSurrogate::validParams()
{
  InputParameters params = SurrogateModel::validParams();
  params.addClassDescription("Computes and evaluates Gaussian Process surrogate model.");
  return params;
}

GaussianProcessSurrogate::GaussianProcessSurrogate(const InputParameters & parameters)
  : SurrogateModel(parameters),
    CovarianceInterface(parameters),
    _gp(declareModelData<StochasticTools::GaussianProcess>("_gp")),
    _training_params(getModelData<torch::Tensor>("_training_params"))
{
}

void
GaussianProcessSurrogate::setupCovariance(UserObjectName covar_name)
{
  if (_gp.getCovarFunctionPtr() != nullptr)
    ::mooseError("Attempting to redefine covariance function using setupCovariance.");
  _gp.linkCovarianceFunction(getCovarianceFunctionByName(covar_name));
}

Real
GaussianProcessSurrogate::evaluate(const std::vector<Real> & x) const
{
  // Overlaod for evaluate to maintain general compatibility. Only returns mean
  Real dummy = 0;
  return this->evaluate(x, dummy);
}

Real
GaussianProcessSurrogate::evaluate(const std::vector<Real> & x, Real & std_dev) const
{
  std::vector<Real> y;
  std::vector<Real> std;
  this->evaluate(x, y, std);
  std_dev = std[0];
  return y[0];
}

void
GaussianProcessSurrogate::evaluate(const std::vector<Real> & x, std::vector<Real> & y) const
{
  // Overlaod for evaluate to maintain general compatibility. Only returns mean
  std::vector<Real> std_dummy;
  this->evaluate(x, y, std_dummy);
}

void
GaussianProcessSurrogate::evaluate(const std::vector<Real> & x,
                                   std::vector<Real> & y,
                                   std::vector<Real> & std) const
{
  const unsigned int n_dims = _training_params.sizes()[1];

  mooseAssert(x.size() == n_dims,
              "Number of parameters provided for evaluation does not match number of parameters "
              "used for training.");
  const unsigned int n_outputs = _gp.getCovarFunction().numOutputs();

  y = std::vector<Real>(n_outputs, 0.0);
  std = std::vector<Real>(n_outputs, 0.0);

  torch::Tensor test_points = torch::empty({1, n_dims}, at::kDouble);
  auto points_accessor = test_points.accessor<Real, 2>();
  for (unsigned int ii = 0; ii < n_dims; ++ii)
    points_accessor[0][ii] = x[ii];

  _gp.getParamStandardizer().getStandardized(test_points);

  torch::Tensor K_train_test =
      torch::empty({_training_params.sizes()[0] * n_outputs, n_outputs}).to(at::kDouble);

  _gp.getCovarFunction().computeCovarianceMatrix(
      K_train_test, _training_params, test_points, false);
  torch::Tensor K_test = torch::empty({n_outputs, n_outputs}).to(at::kDouble);
  _gp.getCovarFunction().computeCovarianceMatrix(K_test, test_points, test_points, true);

  //  Compute the predicted mean value (centered)
  torch::Tensor pred_value = torch::transpose(
      torch::mm(torch::transpose(K_train_test, 0, 1), _gp.getKResultsSolve()), 0, 1);

  //   De-center/scale the value and store for return
  _gp.getDataStandardizer().getDestandardized(pred_value);

  torch::Tensor pred_var =
      K_test - torch::mm(torch::transpose(K_train_test, 0, 1),
                         torch::cholesky_solve(K_train_test, _gp.getKCholeskyDecomp()));

  // Vairance computed, take sqrt for standard deviation, scale up by training data std and store
  torch::Tensor std_dev_mat = torch::sqrt(pred_var); // pred_var.array().sqrt();
  _gp.getDataStandardizer().getDescaled(std_dev_mat);

  for (const auto output_i : make_range(n_outputs))
  {
    y[output_i] = pred_value.data_ptr<Real>()[output_i]; // pred_value(0, output_i);
    std[output_i] = std_dev_mat[output_i][output_i].item<Real>();
  }
}

#endif
