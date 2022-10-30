//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Gaussian.h"
#include "Normal.h"
#include "math.h"
#include "libmesh/utility.h"

registerMooseObject("StochasticToolsApp", Gaussian);

InputParameters
Gaussian::validParams()
{
  InputParameters params = Likelihood::validParams();
  params.addClassDescription("Gaussian likelihood function evaluating the model goodness against experiments.");
  params.addParam<bool>(
      "inferred_noise", false, "Whether noise is inferred from Bayes' rule.");
  params.addParam<bool>(
      "experiment_file", true, "Whether a csv file containing experimental values is provided.");
  params.addParam<bool>(
      "log_likelihood", true, "Compute log-likelihood or likelihood.");
  params.addRequiredParam<std::vector<Real>>("model_pred", "A vector of model predictions.");
  params.addParam<Real>("noise", "User-specified noise value when not inferred from Bayes' rule.");
  params.addParam<std::vector<Real>>("exp_values", "User-specified experimental values when csv file is not provided.");
  return params;
}

Gaussian::Gaussian(const InputParameters & parameters)
  : Likelihood(parameters)
{
}

Real
Gaussian::densityFunction(const std::vector<Real> & x, const Real & noise)
{
  return 1.0 / (std_dev * std::sqrt(2.0 * M_PI)) *
         std::exp(-0.5 * Utility::pow<2>((x - mean) / std_dev));
}

Real
Gaussian::densityFunction(const std::vector<Real> & x) const
{
  return densityFunction(x, _noise);
}
