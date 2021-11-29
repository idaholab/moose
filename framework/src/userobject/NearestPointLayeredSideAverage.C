//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "NearestPointLayeredSideAverage.h"
#include "LayeredSideAverage.h"

registerMooseObject("MooseApp", NearestPointLayeredSideAverage);

InputParameters
NearestPointLayeredSideAverage::validParams()
{
  InputParameters params =
      NearestPointBase<LayeredSideAverage, SideIntegralVariableUserObject>::validParams();

  params.addClassDescription("Compute layered side averages for nearest-point based subdomains");

  return params;
}

NearestPointLayeredSideAverage::NearestPointLayeredSideAverage(const InputParameters & parameters)
  : NearestPointBase<LayeredSideAverage, SideIntegralVariableUserObject>(parameters)
{
}
