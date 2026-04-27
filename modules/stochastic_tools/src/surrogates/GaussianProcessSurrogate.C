//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
    _training_params(getModelData<RealEigenMatrix>("_training_params"))
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
  const unsigned int n_dims = _training_params.cols();

  mooseAssert(x.size() == n_dims,
              "Number of parameters provided for evaluation does not match number of parameters "
              "used for training.");
  const unsigned int n_outputs = _gp.getCovarFunction().numOutputs();
  const unsigned int n_train = _training_params.rows();
  const unsigned int n_virt = _gp.virtualParams().rows();
  const unsigned int n_total = n_train + n_virt;

  y = std::vector<Real>(n_outputs, 0.0);
  std = std::vector<Real>(n_outputs, 0.0);

  RealEigenMatrix test_points(1, n_dims);
  for (unsigned int ii = 0; ii < n_dims; ++ii)
    test_points(0, ii) = x[ii];

  _gp.getParamStandardizer().getStandardized(test_points);

  // Build K_train_test: (n_total x n_outputs) — extended with derivative rows if needed
  RealEigenMatrix K_train_test(n_total * n_outputs, n_outputs);

  // Standard rows: Cov[f(X_train), f(x*)]
  RealEigenMatrix K_ff_test(n_train * n_outputs, n_outputs);
  _gp.getCovarFunction().computeCovarianceMatrix(K_ff_test, _training_params, test_points, false);
  K_train_test.topRows(n_train * n_outputs) = K_ff_test;

  // Derivative rows: Cov[df(x_d^j)/dx_{k_j}, f(x*)] — only for single-output GP
  if (n_virt > 0)
  {
    for (unsigned int j = 0; j < n_virt; ++j)
    {
      RealEigenMatrix xd_j = _gp.virtualParams().row(j);
      RealEigenMatrix K_df_j(1, 1);
      _gp.getCovarFunction().computeCovarianceDf(
          K_df_j, xd_j, test_points, _gp.virtualDerivDims()[j]);
      K_train_test(n_train + j, 0) = K_df_j(0, 0);
    }
  }

  // Self-covariance at test point
  RealEigenMatrix K_test(n_outputs, n_outputs);
  _gp.getCovarFunction().computeCovarianceMatrix(K_test, test_points, test_points, true);

  // Predicted mean in standardized z-space
  RealEigenMatrix pred_value = (K_train_test.transpose() * _gp.getKResultsSolve()).transpose();

  // De-standardize to z-space (post-link, pre-inverse-link)
  _gp.getDataStandardizer().getDestandardized(pred_value);

  // Posterior variance: K_** - k_*^T K_aug^{-1} k_*
  RealEigenMatrix pred_var =
      K_test - (K_train_test.transpose() * _gp.getKCholeskyDecomp().solve(K_train_test));

  RealEigenMatrix std_dev_mat = pred_var.array().sqrt();
  _gp.getDataStandardizer().getDescaled(std_dev_mat);

  for (const auto output_i : make_range(n_outputs))
  {
    const Real mu_z = pred_value(0, output_i);
    const Real sigma_z = std_dev_mat(output_i, output_i);

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
