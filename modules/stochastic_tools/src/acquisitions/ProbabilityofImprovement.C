//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ProbabilityofImprovement.h"
#include "Normal.h"
#include <cmath>

registerMooseObject("StochasticToolsApp", ProbabilityofImprovement);

InputParameters
ProbabilityofImprovement::validParams()
{
  InputParameters params = ParallelAcquisitionFunctionBase::validParams();
  params.addClassDescription("Probability of improvement acquisition function.");
  return params;
}

ProbabilityofImprovement::ProbabilityofImprovement(const InputParameters & parameters)
  : ParallelAcquisitionFunctionBase(parameters)
{
}

void
ProbabilityofImprovement::computeAcquisition(std::vector<Real> & acq,
                                           const std::vector<Real> & gp_mean,
                                           const std::vector<Real> & gp_std,
                                           const std::vector<std::vector<Real>> & /*test_inputs*/,
                                           const std::vector<std::vector<Real>> & /*train_inputs*/,
                                           const std::vector<Real> & generic) const
{
  for (unsigned int i = 0; i < gp_mean.size(); ++i)
    acq[i] = Normal::cdf(gp_mean[i] - generic[0], 0.0, gp_std[i]);
}
