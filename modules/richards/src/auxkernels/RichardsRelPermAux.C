/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

//  This post processor returns the relative permeability of a region.
//
#include "RichardsRelPermAux.h"

template <>
InputParameters
validParams<RichardsRelPermAux>()
{
  InputParameters params = validParams<AuxKernel>();
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
