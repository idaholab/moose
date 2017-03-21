/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

//  This post processor returns the derivative of density wrt pressure
//
#include "RichardsDensityPrimeAux.h"

template <>
InputParameters
validParams<RichardsDensityPrimeAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("pressure_var", "The variable that represents the pressure");
  params.addRequiredParam<UserObjectName>("density_UO",
                                          "Name of user object that defines the density.");
  params.addClassDescription("auxillary variable which is d(density)/dp");
  return params;
}

RichardsDensityPrimeAux::RichardsDensityPrimeAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _pressure_var(coupledValue("pressure_var")),
    _density_UO(getUserObject<RichardsDensity>("density_UO"))
{
}

Real
RichardsDensityPrimeAux::computeValue()
{
  return _density_UO.ddensity(_pressure_var[_qp]);
}
