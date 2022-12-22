//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TruncatedGaussianIID.h"
#include "TruncatedNormal.h"

registerMooseObject("StochasticToolsApp", TruncatedGaussianIID);

InputParameters
TruncatedGaussianIID::validParams()
{
  InputParameters params = GaussianIID::validParams();
  params.addClassDescription("TruncatedGaussianIID likelihood function evaluating the model "
                             "goodness against experiments.");
  params.addRequiredParam<std::vector<Real>>("lower_bound", "Lower bounds.");
  params.addRequiredParam<std::vector<Real>>("upper_bound", "Upper bounds.");
  return params;
}

TruncatedGaussianIID::TruncatedGaussianIID(const InputParameters & parameters)
  : GaussianIID(parameters),
    _lb(getParam<std::vector<Real>>("lower_bound")),
    _ub(getParam<std::vector<Real>>("upper_bound"))
{
  if (_lb.size() != _ub.size())
    mooseError("The number of lower and upper bounds should be equal.");
}

Real
TruncatedGaussianIID::reqFunction(const std::vector<Real> & exp,
                                  const std::vector<std::vector<Real>> & model,
                                  const std::vector<Real> & weights,
                                  const Real & noise,
                                  const bool & log_likelihood) const
{
  if (_lb.size() != model.size())
    mooseError("The number of lower/upper bounds and models should be equal.");
  Real result = 0.0;
  for (unsigned i = 0; i < exp.size(); ++i)
  {
    Real tmp = 0.0;
    for (unsigned j = 0; j < model.size(); ++j)
      tmp += weights[j] * TruncatedNormal::pdf(exp[i], model[j][i], noise, _lb[j], _ub[j]);
    result += std::log(tmp);
  }

  result = log_likelihood ? result : std::exp(result);
  return result;
}
