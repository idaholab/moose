//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideFVFluxBCIntegral.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("MooseApp", SideFVFluxBCIntegral);


InputParameters
SideFVFluxBCIntegral::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params.addRequiredParam<std::vector<std::string>>("fvbcs");
  params.addClassDescription(
      "Computes the side integral of different finite volume flux boundary conditions.");
  return params;
}

SideFVFluxBCIntegral::SideFVFluxBCIntegral(
    const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
  _bc_names(getParam<std::vector<std::string>>("fvbcs"))
{
}

Real
SideFVFluxBCIntegral::computeFaceInfoIntegral(const FaceInfo * const fi)
{
  return 0.0;
}

Real
SideFVFluxBCIntegral::computeQpIntegral()
{
  return 0.0;
}
