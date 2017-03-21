/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CNSFVMachAux.h"

template <>
InputParameters
validParams<CNSFVMachAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addClassDescription("An aux kernel for calculating Mach number.");
  return params;
}

CNSFVMachAux::CNSFVMachAux(const InputParameters & parameters)
  : AuxKernel(parameters), _mach(getMaterialProperty<Real>("mach_number"))
{
}

Real
CNSFVMachAux::computeValue()
{
  return _mach[_qp];
}
