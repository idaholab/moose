//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThresholdElementSubdomainModifier.h"

InputParameters
ThresholdElementSubdomainModifier::validParams()
{
  InputParameters params = ElementSubdomainModifier::validParams();

  params.addRequiredParam<Real>("threshold",
                                "The value above (or below) which to change the element subdomain");
  params.addParam<MooseEnum>("criterion_type",
                             MooseEnum("BELOW EQUAL ABOVE", "ABOVE"),
                             "Criterion to use for the threshold");
  params.addRequiredParam<SubdomainID>("subdomain_id",
                                       "The subdomain ID of the element when the criterion is met");
  params.addParam<SubdomainID>(
      "complement_subdomain_id",
      "The subdomain ID of the element when the criterion is not met. If not provided, the element "
      "subdomain ID will not be modified if the criterion is not met.");
  return params;
}

ThresholdElementSubdomainModifier::ThresholdElementSubdomainModifier(
    const InputParameters & parameters)
  : ElementSubdomainModifier(parameters),
    _threshold(getParam<Real>("threshold")),
    _criterion_type(getParam<MooseEnum>("criterion_type").getEnum<CriterionType>()),
    _subdomain_id(getParam<SubdomainID>("subdomain_id")),
    _complement_subdomain_id(isParamValid("complement_subdomain_id")
                                 ? getParam<SubdomainID>("complement_subdomain_id")
                                 : std::numeric_limits<SubdomainID>::max())
{
}

SubdomainID
ThresholdElementSubdomainModifier::computeSubdomainID()
{
  Real criterion = computeValue();

  bool criterion_met = false;
  switch (_criterion_type)
  {
    case CriterionType::Equal:
      criterion_met = MooseUtils::absoluteFuzzyEqual(criterion - _threshold, 0);
      break;

    case CriterionType::Below:
      criterion_met = criterion < _threshold;
      break;

    case CriterionType::Above:
      criterion_met = criterion > _threshold;
      break;
  }

  return criterion_met ? _subdomain_id : _complement_subdomain_id;
}
