//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LMC.h"
#include "MooseRandom.h"
#include "MathUtils.h"

registerMooseObject("StochasticToolsApp", LMC);

InputParameters
LMC::validParams()
{
  InputParameters params = CovarianceFunctionBase::validParams();
  params.addClassDescription("Covariance function for multioutput Gaussian Processes based on the "
                             "Linear Model of Coregionalization (LMC).");
  params.addParam<unsigned int>(
      "num_latent_funcs", 1., "The number of latent functions for the expansion of the outputs.");
  params.makeParamRequired<unsigned int>("num_outputs");
  params.makeParamRequired<std::vector<UserObjectName>>("covariance_functions");
  return params;
}

LMC::LMC(const InputParameters & parameters)
  : CovarianceFunctionBase(parameters),
    _num_expansion_terms(getParam<unsigned int>("num_latent_funcs"))
{
  // We use a random number generator to obtain the initial guess for the
  // hyperparams
  MooseRandom generator_latent;
  generator_latent.seed(0, 1980);

  // First add and initialize the a A coefficients in the (aa^T+lambda*I) matrix
  for (const auto exp_i : make_range(_num_expansion_terms))
  {
    const std::string a_coeff_name = "acoeff_" + std::to_string(exp_i);
    const std::string a_coeff_name_with_prefix = _name + ":" + a_coeff_name;
    _a_coeffs.push_back(
        &addVectorRealHyperParameter(a_coeff_name, std::vector(_num_outputs, 1.0), true));

    auto & acoeff_vector = _hp_map_vector_real[a_coeff_name_with_prefix];
    for (const auto out_i : make_range(_num_outputs))
      acoeff_vector[out_i] = 3.0 * generator_latent.rand(0) + 1.0;
  }

  // Then add and initialize the lambda coefficients in the (aa^T+lambda*I) matrix
  for (const auto exp_i : make_range(_num_expansion_terms))
  {
    const std::string lambda_name = "lambda_" + std::to_string(exp_i);
    const std::string lambda_name_with_prefix = _name + ":" + lambda_name;
    _lambdas.push_back(
        &addVectorRealHyperParameter(lambda_name, std::vector(_num_outputs, 1.0), true));

    auto & lambda_vector = _hp_map_vector_real[lambda_name_with_prefix];
    for (const auto out_i : make_range(_num_outputs))
      lambda_vector[out_i] = 3.0 * generator_latent.rand(0) + 1.0;
  }
}

void
LMC::computeCovarianceMatrix(RealEigenMatrix & K,
                             const RealEigenMatrix & x,
                             const RealEigenMatrix & xp,
                             const bool is_self_covariance) const
{
  // Create temporary vectors for constructing the covariance matrix
  RealEigenMatrix K_params = RealEigenMatrix::Zero(x.rows(), xp.rows());
  RealEigenMatrix B = RealEigenMatrix::Zero(_num_outputs, _num_outputs);
  K = RealEigenMatrix::Zero(x.rows() * _num_outputs, xp.rows() * _num_outputs);
  RealEigenMatrix K_working =
      RealEigenMatrix::Zero(x.rows() * _num_outputs, xp.rows() * _num_outputs);

  // For every expansion term we add the contribution to the covariance matrix
  for (const auto exp_i : make_range(_num_expansion_terms))
  {
    _covariance_functions[exp_i]->computeCovarianceMatrix(K_params, x, xp, is_self_covariance);
    computeBMatrix(B, exp_i);
    MathUtils::kron(K_working, B, K_params);
    K += K_working;
  }
}

bool
LMC::computedKdhyper(RealEigenMatrix & dKdhp,
                     const RealEigenMatrix & x,
                     const std::string & hyper_param_name,
                     unsigned int ind) const
{
  // Early return in the paramter name is longer than the expected [name] prefix.
  // We prefix the parameter names with the name of the covariance function.
  if (name().length() + 1 > hyper_param_name.length())
    return false;

  // Strip the prefix from the given parameter name
  const std::string name_without_prefix = hyper_param_name.substr(name().length() + 1);

  // Check if the parameter is tunable
  if (_tunable_hp.find(hyper_param_name) != _tunable_hp.end())
  {
    const std::string acoeff_prefix = "acoeff_";
    const std::string lambda_prefix = "lambda_";

    // Allocate storage for the factors of the total gradient matrix
    RealEigenMatrix dBdhp = RealEigenMatrix::Zero(_num_outputs, _num_outputs);
    RealEigenMatrix K_params = RealEigenMatrix::Zero(x.rows(), x.rows());

    if (name_without_prefix.find(acoeff_prefix) != std::string::npos)
    {
      // Automatically grab the expansion index
      const int number = std::stoi(name_without_prefix.substr(acoeff_prefix.length()));
      computeAGradient(dBdhp, number, ind);
      _covariance_functions[number]->computeCovarianceMatrix(K_params, x, x, true);
    }
    else if (name_without_prefix.find(lambda_prefix) != std::string::npos)
    {
      // Automatically grab the expansion index
      const int number = std::stoi(name_without_prefix.substr(lambda_prefix.length()));
      computeLambdaGradient(dBdhp, number, ind);
      _covariance_functions[number]->computeCovarianceMatrix(K_params, x, x, true);
    }
    MathUtils::kron(dKdhp, dBdhp, K_params);
    return true;
  }
  else
  {
    // Allocate storage for the matrix factors
    RealEigenMatrix B_tmp = RealEigenMatrix::Zero(_num_outputs, _num_outputs);
    RealEigenMatrix B = RealEigenMatrix::Zero(_num_outputs, _num_outputs);
    RealEigenMatrix dKdhp_sub = RealEigenMatrix::Zero(x.rows(), x.rows());

    // First, check the dependent covariances
    bool found = false;
    for (const auto dependent_covar : _covariance_functions)
      if (!found)
        found = dependent_covar->computedKdhyper(dKdhp_sub, x, hyper_param_name, ind);

    if (!found)
      mooseError("Hyperparameter ", hyper_param_name, "not found!");

    // Then we compute the output covariance
    for (const auto exp_i : make_range(_num_expansion_terms))
    {
      computeBMatrix(B_tmp, exp_i);
      B += B_tmp;
    }

    MathUtils::kron(dKdhp, B, dKdhp_sub);

    return true;
  }

  return false;
}

void
LMC::computeBMatrix(RealEigenMatrix & Bmat, const unsigned int exp_i) const
{
  const auto & a_coeffs = *_a_coeffs[exp_i];
  const auto & lambda_coeffs = *_lambdas[exp_i];

  for (const auto row_i : make_range(_num_outputs))
    for (const auto col_i : make_range(_num_outputs))
    {
      Bmat(row_i, col_i) = a_coeffs[row_i] * a_coeffs[col_i];
      if (row_i == col_i)
        Bmat(row_i, col_i) += lambda_coeffs[col_i];
    }
}

void
LMC::computeAGradient(RealEigenMatrix & grad,
                      const unsigned int exp_i,
                      const unsigned int index) const
{
  const auto & a_coeffs = *_a_coeffs[exp_i];
  // Add asserts here
  grad.setZero();
  for (const auto col_i : make_range(_num_outputs))
    grad(index, col_i) = a_coeffs[col_i];
  const RealEigenMatrix transpose = grad.transpose();
  grad = grad + transpose;
}

void
LMC::computeLambdaGradient(RealEigenMatrix & grad,
                           const unsigned int /*exp_i*/,
                           const unsigned int index) const
{
  grad.setZero();
  grad(index, index) = 1.0;
}
