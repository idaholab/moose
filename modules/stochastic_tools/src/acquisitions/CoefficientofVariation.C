//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoefficientofVariation.h"
#include <cmath>

registerMooseObject("StochasticToolsApp", CoefficientofVariation);

InputParameters
CoefficientofVariation::validParams()
{
  InputParameters params = ParallelAcquisitionFunctionBase::validParams();
  params.addClassDescription("Coefficient of variation acquisition function.");
  return params;
}

CoefficientofVariation::CoefficientofVariation(const InputParameters & parameters)
  : ParallelAcquisitionFunctionBase(parameters)
{
}

void
CoefficientofVariation::computeAcquisition(std::vector<Real> & acq,
                                         const std::vector<Real> & gp_mean,
                                         const std::vector<Real> & gp_std,
                                         const std::vector<std::vector<Real>> & /*test_inputs*/,
                                         const std::vector<std::vector<Real>> & /*train_inputs*/,
                                         const std::vector<Real> & /*generic*/) const
{
  for (unsigned int i = 0; i < gp_mean.size(); ++i)
    acq[i] = gp_std[i] / gp_mean[i];
}
