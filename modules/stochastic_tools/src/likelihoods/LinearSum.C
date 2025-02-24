//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearSum.h"

registerMooseObject("StochasticToolsApp", LinearSum);

InputParameters
LinearSum::validParams()
{
  InputParameters params = LikelihoodFunctionBase::validParams();
  params.addClassDescription(
      "LinearSum function evaluating the model goodness against experiments.");
  return params;
}

LinearSum::LinearSum(const InputParameters & parameters) : Gaussian(parameters) {}

Real
LinearSum::function(const std::vector<Real> & exp,
                    const std::vector<Real> & model,
                    const Real & noise)
{
  Real result = 0.0;
  Real val1;
  for (unsigned i = 0; i < exp.size(); ++i)
  {
    val1 = Utility::pow<2>((exp[i] - model[i]) / noise);
    result += val1;
  }
  return result;
}

Real
LinearSum::function(const std::vector<Real> & x) const
{
  return function(_exp_values, x, _noise);
}
