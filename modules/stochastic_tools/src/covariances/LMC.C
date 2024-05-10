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
    _num_expansion_terms(getParam<unsigned int>("num_latent_funcs")),
    _num_outputs(getParam<unsigned int>("num_outputs"))
{
  MooseRandom generator_latent;
  generator_latent.seed(0, 1980);

  for (const auto exp_i : make_range(_num_expansion_terms))
  {
    const std::string a_coeff_name = "a_" + std::to_string(exp_i);
    _a_coeffs.push_back(
        &addVectorRealHyperParameter(a_coeff_name, std::vector(_num_outputs, 1.0), true));

    auto & vector = _hp_map_vector_real[a_coeff_name];
    for (const auto out_i : make_range(_num_outputs))
      vector[out_i] = 3.0 * generator_latent.rand(0) + 1.0;

    const std::string lambda_name = "lambda_" + std::to_string(exp_i);
    _lambdas.push_back(
        &addVectorRealHyperParameter(lambda_name, std::vector(_num_outputs, 1.0), true));

    vector = _hp_map_vector_real[lambda_name];
    for (const auto out_i : make_range(_num_outputs))
      vector[out_i] = 3.0 * generator_latent.rand(0) + 1.0;
  }
}

void
LMC::computeCovarianceMatrix(RealEigenMatrix & K,
                             const RealEigenMatrix & x,
                             const RealEigenMatrix & xp,
                             const bool is_self_covariance) const
{
  RealEigenMatrix K_params = RealEigenMatrix::Zero(x.rows(), xp.rows());
  RealEigenMatrix B = RealEigenMatrix::Zero(_num_outputs, _num_outputs);

  for (const auto exp_i : make_range(_num_expansion_terms))
  {
    _covariance_functions[exp_i]->computeCovarianceMatrix(K_params, x, xp, is_self_covariance);
    computeBMatrix(B, exp_i);
    MathUtils::kron(K, B, K_params);
  }
}

void
LMC::computedKdhyper(RealEigenMatrix & /*dKdhp*/,
                     const RealEigenMatrix & /*x*/,
                     const std::string & /*hyper_param_name*/,
                     unsigned int /*ind*/) const
{
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
        Bmat(row_i, col_i) + lambda_coeffs[col_i];
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
  for (const auto row_i : make_range(_num_outputs))
    for (const auto col_i : make_range(_num_outputs))
    {
      grad(index, col_i) = a_coeffs[col_i];
    }
  grad += grad.transpose();
}

void
LMC::computeLambdaGradient(RealEigenMatrix & grad,
                           const unsigned int /*exp_i*/,
                           const unsigned int index) const
{
  grad.setZero();
  grad(index, index) = 1.0;
}
