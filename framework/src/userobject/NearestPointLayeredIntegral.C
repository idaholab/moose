//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "NearestPointLayeredIntegral.h"
#include "LayeredIntegral.h"

registerMooseObject("MooseApp", NearestPointLayeredIntegral);

defineLegacyParams(NearestPointLayeredIntegral);

InputParameters
NearestPointLayeredIntegral::validParams()
{
  InputParameters params =
      nearestPointBaseValidParams<LayeredIntegral, ElementIntegralVariableUserObject>();

  return params;
}

NearestPointLayeredIntegral::NearestPointLayeredIntegral(const InputParameters & parameters)
  : NearestPointBase<LayeredIntegral, ElementIntegralVariableUserObject>(parameters)
{
}
