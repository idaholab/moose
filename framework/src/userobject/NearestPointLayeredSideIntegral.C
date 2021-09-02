//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NearestPointLayeredSideIntegral.h"
#include "LayeredSideIntegral.h"

registerMooseObject("MooseApp", NearestPointLayeredSideIntegral);

defineLegacyParams(NearestPointLayeredSideIntegral);

InputParameters
NearestPointLayeredSideIntegral::validParams()
{
  InputParameters params =
      nearestPointBaseValidParams<LayeredSideIntegral, SideIntegralVariableUserObject>();

  params.addClassDescription("Compute layered side integrals for nearest-point based sidesets");

  return params;
}

NearestPointLayeredSideIntegral::NearestPointLayeredSideIntegral(const InputParameters & parameters)
  : NearestPointBase<LayeredSideIntegral, SideIntegralVariableUserObject>(parameters)
{
}
