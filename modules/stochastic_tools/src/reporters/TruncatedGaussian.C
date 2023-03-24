//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TruncatedGaussian.h"
#include "TruncatedNormal.h"

registerMooseObject("StochasticToolsApp", TruncatedGaussian);

InputParameters
TruncatedGaussian::validParams()
{
  InputParameters params = Gaussian::validParams();
  params.addClassDescription(
      "TruncatedGaussian likelihood function evaluating the model goodness against experiments.");
  params.addRequiredParam<Real>("lb", "Lower bound for the quantity of interest.");
  params.addRequiredParam<Real>("ub", "Upper bound for the quantity of interest.");
  return params;
}

TruncatedGaussian::TruncatedGaussian(const InputParameters & parameters)
  : Gaussian(parameters), _lb(getParam<Real>("lb")), _ub(getParam<Real>("ub"))
{
  if (!(_lb < _ub))
    mooseError("The specified lower bound should be less than the upper bound.");
}

Real
TruncatedGaussian::function(const std::vector<Real> & exp,
                            const std::vector<Real> & model,
                            const Real & noise,
                            const Real & lb,
                            const Real & ub,
                            const bool & log_likelihood)
{
  Real result = 0.0;
  for (unsigned i = 0; i < exp.size(); ++i)
    result += std::log(TruncatedNormal::pdf(exp[i], model[i], noise, lb, ub));
  if (!log_likelihood)
    result = std::exp(result);
  return result;
}

Real
TruncatedGaussian::function(const std::vector<Real> & x) const
{
  return function(_exp_values, x, _noise, _lb, _ub, _log_likelihood);
}
