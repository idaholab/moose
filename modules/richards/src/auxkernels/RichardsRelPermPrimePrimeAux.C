/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

//  This post processor returns d^2(relperm)/d(Seff)^2
//
#include "RichardsRelPermPrimePrimeAux.h"

template <>
InputParameters
validParams<RichardsRelPermPrimePrimeAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("seff_var", "The variable that represents the effective saturation");
  params.addRequiredParam<UserObjectName>(
      "relperm_UO", "Name of user object that defines the relative permeability.");
  params.addClassDescription("auxillary variable which is d^2(relative permeability)/dSeff^2");
  return params;
}

RichardsRelPermPrimePrimeAux::RichardsRelPermPrimePrimeAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _seff_var(coupledValue("seff_var")),
    _relperm_UO(getUserObject<RichardsRelPerm>("relperm_UO"))
{
}

Real
RichardsRelPermPrimePrimeAux::computeValue()
{
  return _relperm_UO.d2relperm(_seff_var[_qp]);
}
