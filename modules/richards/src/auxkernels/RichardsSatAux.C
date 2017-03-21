/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

//  This post processor returns the saturation of a region.
//
#include "RichardsSatAux.h"

template <>
InputParameters
validParams<RichardsSatAux>()
{
  InputParameters params = validParams<AuxKernel>();
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
