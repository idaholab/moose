//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LMC.h"
#include <cmath>
#include "MooseRandom.h"

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

  const auto & covar_function_names = getParam<std::vector<UserObjectName>>("covariance_functions");

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
LMC::computeCovarianceMatrix(RealEigenMatrix & /*K*/,
                             const RealEigenMatrix & /*x*/,
                             const RealEigenMatrix & /*xp*/,
                             const bool /*is_self_covariance*/) const
{
}

void
LMC::computedKdhyper(RealEigenMatrix & /*dKdhp*/,
                     const RealEigenMatrix & /*x*/,
                     const std::string & /*hyper_param_name*/,
                     unsigned int /*ind*/) const
{
}
