//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SBMElementClassificationInterface.h"

#include "MooseObject.h"

InputParameters
SBMElementClassificationInterface::validParams()
{
  InputParameters params = emptyInputParameters();

  params.addRangeCheckedParam<Real>(
      "lambda",
      0.5,
      "lambda >= 0 & lambda <= 1",
      "Threshold applied to the fraction outside the retained domain for a partial element. This "
      "parameter cannot be set when 'mark_intercepted' is true.");

  params.addRangeCheckedParam<int>(
      "qrule_order",
      9,
      "qrule_order >= 0 & qrule_order <= 10",
      "Quadrature order used to estimate the retained-domain fraction.");

  params.addParam<bool>(
      "mark_intercepted", false, "Whether to assign a dedicated subdomain ID when intercepted.");
  params.addParam<SubdomainID>("subdomain_id_intercepted",
                               Moose::INVALID_BLOCK_ID,
                               "Subdomain ID assigned to intercepted elements.");

  return params;
}

SBMElementClassificationInterface::SBMElementClassificationInterface(
    const MooseObject * moose_object)
  : _lambda(moose_object->getParam<Real>("lambda")),
    _qrule_order(static_cast<Order>(moose_object->getParam<int>("qrule_order"))),
    _intercepted_subdomain_policy{moose_object->getParam<bool>("mark_intercepted"),
                                  moose_object->getParam<SubdomainID>("subdomain_id_intercepted")}
{
  if (_intercepted_subdomain_policy.mark_intercepted &&
      _intercepted_subdomain_policy.subdomain_id == Moose::INVALID_BLOCK_ID)
    moose_object->paramError("subdomain_id_intercepted",
                             "This parameter must be specified when 'mark_intercepted' is true.");

  if (_intercepted_subdomain_policy.mark_intercepted && moose_object->isParamSetByUser("lambda"))
    moose_object->paramError("lambda",
                             "This parameter cannot be set when 'mark_intercepted' is true.");
}
