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
#include "RichardsSatPrimeAux.h"

registerMooseObject("RichardsApp", RichardsSatPrimeAux);

InputParameters
RichardsSatPrimeAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredCoupledVar("seff_var", "Variable that is the effective saturation");
  params.addRequiredParam<UserObjectName>("sat_UO", "Name of user object that defines saturation.");
  params.addClassDescription("auxillary variable which is saturation");
  return params;
}

RichardsSatPrimeAux::RichardsSatPrimeAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _seff_var(coupledValue("seff_var")),
    _sat_UO(getUserObject<RichardsSat>("sat_UO"))
{
}

Real
RichardsSatPrimeAux::computeValue()
{
  return _sat_UO.dsat(_seff_var[_qp]);
}
