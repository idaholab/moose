//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#ifdef MOOSE_LIBTORCH_ENABLED

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
  const unsigned int n_train = _training_params.size(0);
  const unsigned int n_virt = _gp.virtualParams().size(0);
  const unsigned int n_total = n_train + n_virt;

  y = std::vector<Real>(n_outputs, 0.0);
  std = std::vector<Real>(n_outputs, 0.0);

  const auto options = _training_params.options().dtype(at::kDouble);

  torch::Tensor test_points = torch::empty({1, n_dims}, at::kDouble);
  auto points_accessor = test_points.accessor<Real, 2>();
  for (unsigned int ii = 0; ii < n_dims; ++ii)
    points_accessor[0][ii] = x[ii];
  test_points = test_points.to(options.device());

  _gp.getParamStandardizer().getStandardized(test_points);

  // Build K_train_test: (n_total x n_outputs) — extended with derivative rows if needed
  torch::Tensor K_train_test =
      torch::empty({(long)(n_total * n_outputs), (long)n_outputs}, options);

  // Standard rows: Cov[f(X_train), f(x*)]
  torch::Tensor K_ff_test; // computeCovarianceMatrix reassigns its output argument
  _gp.getCovarFunction().computeCovarianceMatrix(K_ff_test, _training_params, test_points, false);
  K_train_test.narrow(0, 0, n_train * n_outputs).copy_(K_ff_test);

  // Derivative rows: Cov[df(x_d^j)/dx_{k_j}, f(x*)] — only for single-output GP
  if (n_virt > 0)
  {
    const auto virtual_params = _gp.virtualParams();
    for (unsigned int j = 0; j < n_virt; ++j)
    {
      torch::Tensor K_df_j; // computeCovarianceDf reassigns its output argument
      _gp.getCovarFunction().computeCovarianceDf(
          K_df_j, virtual_params.narrow(0, j, 1), test_points, _gp.virtualDerivDims()[j]);
      K_train_test.narrow(0, n_train + j, 1).copy_(K_df_j);
    }
  }

  // Self-covariance at test point
  torch::Tensor K_test = torch::empty({n_outputs, n_outputs}, options);
  _gp.getCovarFunction().computeCovarianceMatrix(K_test, test_points, test_points, true);

  //  Compute the predicted mean value (centered)
  torch::Tensor pred_value = torch::transpose(
      torch::mm(torch::transpose(K_train_test, 0, 1), _gp.getKResultsSolve()), 0, 1);

  //   De-center/scale the value and store for return
  _gp.getDataStandardizer().getDestandardized(pred_value);

  torch::Tensor pred_var =
      K_test - torch::mm(torch::transpose(K_train_test, 0, 1),
                         torch::cholesky_solve(K_train_test, _gp.getKCholeskyDecomp()));

  // Only the marginal variances are returned. Clamp tiny negative roundoff before sqrt.
  torch::Tensor std_dev_vec =
      torch::sqrt(torch::clamp_min(torch::diagonal(pred_var), 0.0)).unsqueeze(0);
  _gp.getDataStandardizer().getDescaled(std_dev_vec);
  const auto std_dev_cpu = LibtorchUtils::toCPUContiguous(std_dev_vec);
  const auto pred_value_cpu = LibtorchUtils::toCPUContiguous(pred_value);
  auto std_accessor = std_dev_cpu.accessor<Real, 2>();
  auto pred_value_accessor = pred_value_cpu.accessor<Real, 2>();

  for (const auto output_i : make_range(n_outputs))
  {
    const Real mu_z = pred_value_accessor[0][output_i];
    const Real sigma_z = std_accessor[0][output_i];

    if (_gp.hasLinkFunction())
    {
      // Apply inverse link to mean; propagate uncertainty via delta method
      y[output_i] = _gp.applyInvLink(mu_z);
      std[output_i] = std::abs(_gp.invLinkDeriv(mu_z)) * sigma_z;
    }
    else
    {
      y[output_i] = mu_z;
      std[output_i] = sigma_z;
    }
  }
}

#endif
