//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "UFunction.h"
#include <cmath>

registerMooseObject("StochasticToolsApp", UFunction);

InputParameters
UFunction::validParams()
{
  InputParameters params = ParallelAcquisitionFunctionBase::validParams();
  params.addClassDescription("U-function acquisition function for rare events analysis.");
  return params;
}

UFunction::UFunction(const InputParameters & parameters)
  : ParallelAcquisitionFunctionBase(parameters)
{
}

void
UFunction::computeAcquisition(std::vector<Real> & acq,
                              const std::vector<Real> & gp_mean,
                              const std::vector<Real> & gp_std,
                              const std::vector<std::vector<Real>> & /*test_inputs*/,
                              const std::vector<std::vector<Real>> & /*train_inputs*/,
                              const std::vector<Real> & /*generic*/) const
{
  for (unsigned int i = 0; i < gp_mean.size(); ++i)
    acq[i] = std::abs(gp_mean[i]) / gp_std[i];
}
