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

registerMooseObject("StochasticToolsApp", LMC);

InputParameters
LMC::validParams()
{
  InputParameters params = CovarianceFunctionBase::validParams();
  params.addClassDescription("Covariance function for multioutput Gaussian Processes based on the "
                             "Linear Model of Coregionalization (LMC).");
  params.addParam<unsigned int>(
      "num_latent_funcs", 1., "The number of latent functions for the expansion of the outputs.");
  params.addParam<std::vector<UserObjectName>>("covariance_functions",
                                               "The covariance function for each expansion term.");
  params.addRequiredParam<unsigned int>(
      "num_outputs", "The number of outputs expected for this covariance function.");
  return params;
}

LMC::LMC(const InputParameters & parameters)
  : CovarianceFunctionBase(parameters),
    CovarianceInterface(parameters),
    _num_expansion_terms(getParam<unsigned int>("num_outputs"))
{
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
                     std::string /*hyper_param_name*/,
                     unsigned int /*ind*/) const
{
}
