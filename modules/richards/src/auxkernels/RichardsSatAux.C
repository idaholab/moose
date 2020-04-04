//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//  This post processor returns the saturation of a region.
//
#include "RichardsSatAux.h"

registerMooseObject("RichardsApp", RichardsSatAux);

InputParameters
RichardsSatAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredCoupledVar("seff_var", "Variable that is the effective saturation");
  params.addRequiredParam<UserObjectName>("sat_UO", "Name of user object that defines saturation.");
  params.addClassDescription("auxillary variable which is saturation");
  return params;
}

RichardsSatAux::RichardsSatAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _seff_var(coupledValue("seff_var")),
    _sat_UO(getUserObject<RichardsSat>("sat_UO"))
{
}

Real
RichardsSatAux::computeValue()
{
  return _sat_UO.sat(_seff_var[_qp]);
}
