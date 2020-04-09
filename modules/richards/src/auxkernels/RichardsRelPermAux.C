//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//  This post processor returns the relative permeability of a region.
//
#include "RichardsRelPermAux.h"

registerMooseObject("RichardsApp", RichardsRelPermAux);

InputParameters
RichardsRelPermAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredCoupledVar("seff_var", "The variable that represents the effective saturation");
  params.addRequiredParam<UserObjectName>(
      "relperm_UO", "Name of user object that defines the relative permeability.");
  params.addClassDescription("auxillary variable which is the relative permeability");
  return params;
}

RichardsRelPermAux::RichardsRelPermAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _seff_var(coupledValue("seff_var")),
    _relperm_UO(getUserObject<RichardsRelPerm>("relperm_UO"))
{
}

Real
RichardsRelPermAux::computeValue()
{
  return _relperm_UO.relperm(_seff_var[_qp]);
}
