/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CNSFVPressureAux.h"

template <>
InputParameters
validParams<CNSFVPressureAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addClassDescription("An aux kernel for calculating pressure.");
  return params;
}

CNSFVPressureAux::CNSFVPressureAux(const InputParameters & parameters)
  : AuxKernel(parameters), _pres(getMaterialProperty<Real>("pressure"))
{
}

Real
CNSFVPressureAux::computeValue()
{
  return _pres[_qp];
}
