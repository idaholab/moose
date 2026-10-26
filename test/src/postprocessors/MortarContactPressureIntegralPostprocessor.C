//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarContactPressureIntegralPostprocessor.h"
#include "MortarContactPressureIntegral.h"

registerMooseObject("MooseTestApp", MortarContactPressureIntegralPostprocessor);

InputParameters
MortarContactPressureIntegralPostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Reports the integral computed by a MortarContactPressureIntegral.");
  params.addRequiredParam<UserObjectName>("integral",
                                          "The MortarContactPressureIntegral user object.");
  return params;
}

MortarContactPressureIntegralPostprocessor::MortarContactPressureIntegralPostprocessor(
    const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _integral(getUserObject<MortarContactPressureIntegral>("integral"))
{
}

Real
MortarContactPressureIntegralPostprocessor::getValue() const
{
  return _integral.integral();
}
