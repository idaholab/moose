//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//  This post processor returns the density of a region.
//
#include "RichardsDensityAux.h"

registerMooseObject("RichardsApp", RichardsDensityAux);

InputParameters
RichardsDensityAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredCoupledVar("pressure_var", "The variable that represents the pressure");
  params.addRequiredParam<UserObjectName>("density_UO",
                                          "Name of user object that defines the density.");
  params.addClassDescription("auxillary variable which is fluid density");
  return params;
}

RichardsDensityAux::RichardsDensityAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _pressure_var(coupledValue("pressure_var")),
    _density_UO(getUserObject<RichardsDensity>("density_UO"))
{
}

Real
RichardsDensityAux::computeValue()
{
  return _density_UO.density(_pressure_var[_qp]);
}
