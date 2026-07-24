//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SBMElementSubdomainModifierBase.h"
#include "MooseUtils.h"

InputParameters
SBMElementSubdomainModifierBase::validParams()
{
  InputParameters params = ElementSubdomainModifier::validParams();

  params.addRangeCheckedParam<Real>(
      "lambda",
      0.5,
      "lambda >= 0 & lambda <= 1",
      "Threshold applied to the inactive fraction of a partially active element.");

  params.addRangeCheckedParam<int>("qrule_order",
                                   9,
                                   "qrule_order >= 0 & qrule_order <= 10",
                                   "Quadrature order used to estimate the active fraction.");

  params.addParam<bool>(
      "mark_intercepted", false, "Whether to assign a dedicated subdomain ID when intercepted.");
  params.addParam<SubdomainID>("subdomain_id_intercepted",
                               Moose::INVALID_BLOCK_ID,
                               "The subdomain ID to assign to intercepted elements.");

  return params;
}

SBMElementSubdomainModifierBase::SBMElementSubdomainModifierBase(const InputParameters & parameters)
  : ElementSubdomainModifier(parameters),
    _lambda(getParam<Real>("lambda")),
    _qrule_order(static_cast<Order>(getParam<int>("qrule_order"))),
    _mark_intercepted(getParam<bool>("mark_intercepted")),
    _subdomain_id_intercepted(getParam<SubdomainID>("subdomain_id_intercepted"))
{
  if (_mark_intercepted && _subdomain_id_intercepted == Moose::INVALID_BLOCK_ID)
    paramError("subdomain_id_intercepted",
               "This parameter must be specified when 'mark_intercepted' is true.");
}

bool
SBMElementSubdomainModifierBase::isInactive(const Real active_fraction, const Real lambda)
{
  if (MooseUtils::absoluteFuzzyEqual(lambda, 0))
    return true;
  if (MooseUtils::absoluteFuzzyEqual(lambda, 1))
    return false;

  return MooseUtils::absoluteFuzzyGreaterThan(1.0 - active_fraction, lambda);
}
