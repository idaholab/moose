//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NearestPointLayeredSideDiffusiveFluxAverage.h"
#include "LayeredSideDiffusiveFluxAverage.h"

registerMooseObject("MooseApp", NearestPointLayeredSideDiffusiveFluxAverage);
registerMooseObjectRenamed("MooseApp",
                           NearestPointLayeredSideFluxAverage,
                           "12/31/2022 24:00",
                           NearestPointLayeredSideDiffusiveFluxAverage);

InputParameters
NearestPointLayeredSideDiffusiveFluxAverage::validParams()
{
  InputParameters params = NearestPointBase<LayeredSideDiffusiveFluxAverage,
                                            SideIntegralVariableUserObject>::validParams();

  params.addClassDescription(
      "Compute layered side diffusive flux averages for nearest-point based subdivisions");

  return params;
}

NearestPointLayeredSideDiffusiveFluxAverage::NearestPointLayeredSideDiffusiveFluxAverage(
    const InputParameters & parameters)
  : NearestPointBase<LayeredSideDiffusiveFluxAverage, SideIntegralVariableUserObject>(parameters)
{
}
