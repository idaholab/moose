//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VarianceImprovementGlobalFit.h"
#include <cmath>

registerMooseObject("StochasticToolsApp", VarianceImprovementGlobalFit);

InputParameters
VarianceImprovementGlobalFit::validParams()
{
  InputParameters params = ParallelAcquisitionFunctionBase::validParams();
  params.addClassDescription("Variance of improvement for global fit (VIGF) by Mohammadi and Challenor 2024.");
  return params;
}

VarianceImprovementGlobalFit::VarianceImprovementGlobalFit(const InputParameters & parameters)
  : ParallelAcquisitionFunctionBase(parameters)
{
}

void
VarianceImprovementGlobalFit::computeAcquisitionInternal(
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
    const Real var = Utility::pow<2>(gp_std[i]);
    const Real bias_sq = Utility::pow<2>(gp_mean[i] - generic[ref_ind]);
    acq[i] = 4.0 * var * bias_sq + 2.0 * Utility::pow<2>(var);
  }
}

void
VarianceImprovementGlobalFit::computeDistance(
    unsigned int & req_index,
    const std::vector<Real> & current_input,
    const std::vector<std::vector<Real>> & train_inputs)
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
