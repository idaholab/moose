//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExpectedImprovement.h"
#include "Normal.h"
#include <cmath>

registerMooseObject("StochasticToolsApp", ExpectedImprovement);

InputParameters
ExpectedImprovement::validParams()
{
  InputParameters params = ParallelAcquisitionFunctionBase::validParams();
  params.addClassDescription("Expected improvement acquisition function.");
  params.addRangeCheckedParam<Real>(
      "tuning", 0.001, "tuning > 0", "Tuning parameter to control exploration vs exploitation.");
  return params;
}

ExpectedImprovement::ExpectedImprovement(const InputParameters & parameters)
  : ParallelAcquisitionFunctionBase(parameters), _tuning(getParam<Real>("tuning"))
{
}

void
ExpectedImprovement::computeAcquisition(std::vector<Real> & acq,
                                        const std::vector<Real> & gp_mean,
                                        const std::vector<Real> & gp_std,
                                        const std::vector<std::vector<Real>> & /*test_inputs*/,
                                        const std::vector<std::vector<Real>> & /*train_inputs*/,
                                        const std::vector<Real> & generic) const
{
  auto maxIt = std::max_element(generic.begin(), generic.end());
  Real z;
  for (unsigned int i = 0; i < gp_mean.size(); ++i)
  {
    z = gp_mean[i] - *maxIt - _tuning;
    acq[i] = (gp_mean[i] - *maxIt) * Normal::cdf(z, 0.0, gp_std[i]) +
             gp_std[i] * Normal::pdf(z, 0.0, gp_std[i]);
  }
}
