//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExpectedImprovementGlobalFit.h"
#include <cmath>

registerMooseObject("StochasticToolsApp", ExpectedImprovementGlobalFit);

InputParameters
ExpectedImprovementGlobalFit::validParams()
{
  InputParameters params = ParallelAcquisitionFunctionBase::validParams();
  params.addClassDescription("Expected improvement for global fit (EIGF) by Lam and Notz 2008.");
  return params;
}

ExpectedImprovementGlobalFit::ExpectedImprovementGlobalFit(const InputParameters & parameters)
  : ParallelAcquisitionFunctionBase(parameters)
{
}

void
ExpectedImprovementGlobalFit::computeAcquisition(
    std::vector<Real> & acq,
    const std::vector<Real> & gp_mean,
    const std::vector<Real> & gp_std,
    const std::vector<std::vector<Real>> & test_inputs,
    const std::vector<std::vector<Real>> & train_inputs,
    const std::vector<Real> & generic) const
{
  unsigned int ref_ind;
  for (unsigned int i = 0; i < test_inputs.size(); ++i)
  {
    computeDistance(ref_ind, test_inputs[i], train_inputs);
    acq[i] = Utility::pow<2>(gp_mean[i] - generic[ref_ind]) + Utility::pow<2>(gp_std[i]);
  }
}

void
ExpectedImprovementGlobalFit::computeDistance(unsigned int & req_index,
                                              const std::vector<Real> & current_input,
                                              const std::vector<std::vector<Real>> & train_inputs) const
{
  Real ref_distance = std::numeric_limits<Real>::max();
  Real distance;
  req_index = 0;
  for (unsigned int i = 0; i < train_inputs.size(); ++i)
  {
    distance = 0.0;
    for (unsigned int j = 0; j < current_input.size(); ++j)
      distance += Utility::pow<2>(current_input[j] - train_inputs[i][j]);
    if (distance <= ref_distance)
    {
      ref_distance = distance;
      req_index = i;
    }
  }
}
