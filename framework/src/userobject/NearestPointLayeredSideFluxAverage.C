//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NearestPointLayeredSideFluxAverage.h"
#include "LayeredSideDiffusiveFluxAverage.h"

registerMooseObject("MooseApp", NearestPointLayeredSideFluxAverage);

InputParameters
NearestPointLayeredSideFluxAverage::validParams()
{
  InputParameters params = NearestPointBase<LayeredSideDiffusiveFluxAverage,
                                            SideIntegralVariableUserObject>::validParams();

  params.addClassDescription(
      "Compute layered side flux averages for nearest-point based subdomains");

  return params;
}

NearestPointLayeredSideFluxAverage::NearestPointLayeredSideFluxAverage(
    const InputParameters & parameters)
  : NearestPointBase<LayeredSideDiffusiveFluxAverage, SideIntegralVariableUserObject>(parameters)
{
}
