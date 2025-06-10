//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GaussianProcessSurrogateTorched.h"
#include "Sampler.h"

#include "CovarianceFunctionBaseTorched.h"

registerMooseObject("StochasticToolsApp", GaussianProcessSurrogateTorched);

InputParameters
GaussianProcessSurrogateTorched::validParams()
{
  InputParameters params = SurrogateModel::validParams();
  params.addClassDescription("Computes and evaluates Gaussian Process surrogate model.");
  return params;
}

GaussianProcessSurrogateTorched::GaussianProcessSurrogateTorched(const InputParameters & parameters)
  : SurrogateModel(parameters),
    CovarianceInterfaceTorched(parameters),
    _gp(declareModelData<StochasticToolsTorched::GaussianProcessTorched>("_gp")),
    _training_params(getModelData<RealEigenMatrix>("_training_params"))
{
}

void
GaussianProcessSurrogateTorched::setupCovariance(UserObjectName covar_name)
{
  if (_gp.getCovarFunctionPtr() != nullptr)
    ::mooseError("Attempting to redefine covariance function using setupCovariance.");
  _gp.linkCovarianceFunction(getCovarianceFunctionByName(covar_name));
}

Real
GaussianProcessSurrogateTorched::evaluate(const std::vector<Real> & x) const
{
  // Overlaod for evaluate to maintain general compatibility. Only returns mean
  Real dummy = 0;
  return this->evaluate(x, dummy);
}

Real
GaussianProcessSurrogateTorched::evaluate(const std::vector<Real> & x, Real & std_dev) const
{
  std::vector<Real> y;
  std::vector<Real> std;
  this->evaluate(x, y, std);
  std_dev = std[0];
  return y[0];
}

void
GaussianProcessSurrogateTorched::evaluate(const std::vector<Real> & x, std::vector<Real> & y) const
{
  // Overlaod for evaluate to maintain general compatibility. Only returns mean
  std::vector<Real> std_dummy;
  this->evaluate(x, y, std_dummy);
}

void
GaussianProcessSurrogateTorched::evaluate(const std::vector<Real> & x,
                                          std::vector<Real> & y,
                                          std::vector<Real> & std) const
{
  const unsigned int n_dims = _training_params.cols();

  mooseAssert(x.size() == n_dims,
              "Number of parameters provided for evaluation does not match number of parameters "
              "used for training.");
  const unsigned int n_outputs = _gp.getCovarFunction().numOutputs();

  y = std::vector<Real>(n_outputs, 0.0);
  std = std::vector<Real>(n_outputs, 0.0);

  RealEigenMatrix test_points(1, n_dims);
  for (unsigned int ii = 0; ii < n_dims; ++ii)
    test_points(0, ii) = x[ii];

  auto options = torch::TensorOptions().dtype(at::kDouble);
  torch::Tensor tensor_points =
      torch::from_blob(test_points.data(), {test_points.rows(), test_points.cols()}, options)
          .to(at::kDouble);

  // make non-const copy
  auto training_copy = _training_params;
  torch::Tensor training_tensor =
      torch::from_blob(training_copy.data(), {training_copy.cols(), training_copy.rows()}, options)
          .clone();
  torch::Tensor training_tensor_transposed = training_tensor.transpose(1, 0);
  _gp.getParamStandardizer().getStandardized(tensor_points);

  auto points_accessor = tensor_points.accessor<Real, 2>();
  for (unsigned int i = 0; i < test_points.cols(); i++)
    test_points(0, i) = points_accessor[0][i];

  torch::Tensor K_train_test =
      torch::empty({_training_params.rows() * n_outputs, n_outputs}).to(at::kDouble);

  std::cout << "K_train_test before:" << std::endl << K_train_test << std::endl;
  _gp.getCovarFunction().computeCovarianceMatrix(
      K_train_test, training_tensor_transposed, tensor_points, false);
  std::cout << "K_train_test after:" << std::endl << K_train_test << std::endl;
  torch::Tensor K_test = torch::empty({n_outputs, n_outputs}).to(at::kDouble);
  std::cout << "K_test before:" << std::endl << K_test << std::endl;
  _gp.getCovarFunction().computeCovarianceMatrix(K_test, tensor_points, tensor_points, true);
  std::cout << "K_test after:" << std::endl << K_test << std::endl;

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
