/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CNSFVSpecificTotalEnthalpyAux.h"

template <>
InputParameters
validParams<CNSFVSpecificTotalEnthalpyAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addClassDescription("An aux kernel for calculating specific total enthalpy.");
  return params;
}

CNSFVSpecificTotalEnthalpyAux::CNSFVSpecificTotalEnthalpyAux(const InputParameters & parameters)
  : AuxKernel(parameters), _enth(getMaterialProperty<Real>("enthalpy"))
{
}

Real
CNSFVSpecificTotalEnthalpyAux::computeValue()
{
  return _enth[_qp];
}
