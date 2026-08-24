//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPLinkFunction.h"
#include "MooseError.h"

#include <cmath>

namespace StochasticTools
{

// LogLinkFunction -----------------------------------------------------------------

Real
LogLinkFunction::forward(Real y) const
{
  return std::log(y - _lb);
}

Real
LogLinkFunction::inverse(Real z) const
{
  return std::exp(z) + _lb;
}

Real
LogLinkFunction::inverseDeriv(Real z) const
{
  return std::exp(z);
}

Real
LogLinkFunction::logJacobian(Real y) const
{
  // log|g'(y)| = log|1/(y-lb)| = -log(y-lb)
  return -std::log(y - _lb);
}

// LogitLinkFunction ---------------------------------------------------------------

Real
LogitLinkFunction::forward(Real y) const
{
  const Real p = (y - _lb) / (_ub - _lb);
  return std::log(p / (1.0 - p));
}

Real
LogitLinkFunction::inverse(Real z) const
{
  return _lb + (_ub - _lb) / (1.0 + std::exp(-z));
}

Real
LogitLinkFunction::inverseDeriv(Real z) const
{
  const Real sig = 1.0 / (1.0 + std::exp(-z));
  return (_ub - _lb) * sig * (1.0 - sig);
}

Real
LogitLinkFunction::logJacobian(Real y) const
{
  // g'(y) = (ub-lb) / ((y-lb)*(ub-y))
  // log|g'(y)| = log(ub-lb) - log(y-lb) - log(ub-y)
  return std::log(_ub - _lb) - std::log(y - _lb) - std::log(_ub - y);
}

// Factory -------------------------------------------------------------------------

std::unique_ptr<GPLinkFunction>
GPLinkFunction::build(GPLinkFunctionType type, Real lb, Real ub)
{
  switch (type)
  {
    case GPLinkFunctionType::Identity:
      return std::make_unique<IdentityLinkFunction>();
    case GPLinkFunctionType::Log:
      return std::make_unique<LogLinkFunction>(lb);
    case GPLinkFunctionType::Logit:
      return std::make_unique<LogitLinkFunction>(lb, ub);
    default:
      mooseError("Unknown GPLinkFunctionType");
  }
}

std::unique_ptr<GPLinkFunction>
GPLinkFunction::buildFromString(const std::string & type_name, Real lb, Real ub)
{
  if (type_name == "identity")
    return build(GPLinkFunctionType::Identity, lb, ub);
  if (type_name == "log")
    return build(GPLinkFunctionType::Log, lb, ub);
  if (type_name == "logit")
    return build(GPLinkFunctionType::Logit, lb, ub);
  mooseError("Unknown link function type '",
             type_name,
             "'. Supported options: 'identity', 'log', 'logit'.");
}

} // namespace StochasticTools
