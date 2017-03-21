/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

//  This post processor returns the 2nd derivative of density wrt pressure
//
#include "RichardsDensityPrimePrimeAux.h"

template <>
InputParameters
validParams<RichardsDensityPrimePrimeAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("pressure_var", "The variable that represents the pressure");
  params.addRequiredParam<UserObjectName>("density_UO",
                                          "Name of user object that defines the density.");
  params.addClassDescription("auxillary variable which is d^2(density)/dp^2");
  return params;
}

RichardsDensityPrimePrimeAux::RichardsDensityPrimePrimeAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _pressure_var(coupledValue("pressure_var")),
    _density_UO(getUserObject<RichardsDensity>("density_UO"))
{
}

Real
RichardsDensityPrimePrimeAux::computeValue()
{
  return _density_UO.d2density(_pressure_var[_qp]);
}
