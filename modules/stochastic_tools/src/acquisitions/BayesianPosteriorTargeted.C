//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BayesianPosteriorTargeted.h"
#include <cmath>

registerMooseObject("StochasticToolsApp", BayesianPosteriorTargeted);

InputParameters
BayesianPosteriorTargeted::validParams()
{
  InputParameters params = ParallelAcquisitionFunctionBase::validParams();
  params.addClassDescription("Bayesian posterior targeted from El Gammal et al. 2023.");
  return params;
}

BayesianPosteriorTargeted::BayesianPosteriorTargeted(const InputParameters & parameters)
  : ParallelAcquisitionFunctionBase(parameters)
{
}

void
BayesianPosteriorTargeted::computeAcquisition(
    std::vector<Real> & acq,
    const std::vector<Real> & gp_mean,
    const std::vector<Real> & gp_std,
    const std::vector<std::vector<Real>> & test_inputs,
    const std::vector<std::vector<Real>> & /*train_inputs*/,
    const std::vector<Real> & /*generic*/) const
{
  Real psi = std::pow(test_inputs[0].size(), -0.85);
  for (unsigned int i = 0; i < test_inputs.size(); ++i)
  {
    acq[i] = std::exp(2.0 * psi * gp_mean[i]) * (std::exp(gp_std[i]) - 1.0);
    if (std::isinf(acq[i]))
      acq[i] = std::numeric_limits<Real>::max();
  }
}
