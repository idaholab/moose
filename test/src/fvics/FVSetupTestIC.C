//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVSetupTestIC.h"

registerMooseObject("MooseTestApp", FVSetupTestIC);

InputParameters
FVSetupTestIC::validParams()
{
  InputParameters params = FVInitialCondition::validParams();
  params.addParam<Real>("value", 0, "The value to impose");
  params.addClassDescription("Constant FV initial condition that counts its initialSetup() calls, "
                             "for testing initial condition setup dispatch.");
  return params;
}

FVSetupTestIC::FVSetupTestIC(const InputParameters & parameters)
  : FVInitialCondition(parameters), _value(getParam<Real>("value"))
{
}

Real
FVSetupTestIC::value(const Point & /*p*/)
{
  return _value;
}
