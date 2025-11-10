//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosCoupledAux.h"

registerKokkosAuxKernel("MooseTestApp", KokkosCoupledAux);

InputParameters
KokkosCoupledAux::validParams()
{
  InputParameters params = AuxKernel::validParams();

  MooseEnum operators("+ - * /", "+");

  params.addRequiredCoupledVar("coupled", "Coupled Value for Calculation");

  params.addParam<Real>(
      "value", 0.0, "A value to use in the binary arithmetic operation of this coupled auxkernel");
  params.addParam<MooseEnum>(
      "operator", operators, "The binary operator to use in the calculation");
  return params;
}

KokkosCoupledAux::KokkosCoupledAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _value(getParam<Real>("value")),
    _operator(getParam<MooseEnum>("operator")),
    _coupled(coupled("coupled")),
    _coupled_val(kokkosCoupledValue("coupled"))
{
}
