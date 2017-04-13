/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

//  This post processor returns the saturation of a region.
//
#include "RichardsSatPrimeAux.h"

template <>
InputParameters
validParams<RichardsSatPrimeAux>()
{
  InputParameters params = validParams<AuxKernel>();
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
