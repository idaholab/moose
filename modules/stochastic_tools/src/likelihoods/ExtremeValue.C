//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExtremeValue.h"

registerMooseObject("StochasticToolsApp", ExtremeValue);

InputParameters
ExtremeValue::validParams()
{
  InputParameters params = Gaussian::validParams();
  params.addClassDescription("Generalized extreme value likelihood function evaluating the model "
                             "goodness against experiments.");
  return params;
}

ExtremeValue::ExtremeValue(const InputParameters & parameters) : Gaussian(parameters) {}

Real
ExtremeValue::function(const std::vector<Real> & exp,
                       const std::vector<Real> & exp_group,
                       const std::vector<Real> & model,
                       const std::vector<Real> & noise,
                       const bool & log_likelihood)
{
  Real result = 0.0;
  unsigned int group_ind;
  for (unsigned i = 0; i < exp.size(); ++i)
  {
    group_ind = exp_group[i];
    Real x = (exp[i] - model[i]) / noise[group_ind];
    Real pdf = std::exp(-(x + std::exp(-x))) / noise[group_ind];
    result += std::log(pdf);
  }
  if (!log_likelihood)
    result = std::exp(result);
  return result;
}

Real
ExtremeValue::function(const std::vector<Real> & x) const
{
  return function(_exp_values, _exp_groups, x, _noise, _log_likelihood);
}
