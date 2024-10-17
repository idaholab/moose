//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledAux.h"

registerMooseObject("MooseTestApp", CoupledAux);
registerMooseObject("MooseTestApp", ADCoupledAux);

template <bool is_ad>
InputParameters
CoupledAuxTempl<is_ad>::validParams()
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

template <bool is_ad>
CoupledAuxTempl<is_ad>::CoupledAuxTempl(const InputParameters & parameters)
  : AuxKernel(parameters),
    _value(getParam<Real>("value")),
    _operator(getParam<MooseEnum>("operator")),
    _coupled(coupled("coupled")),
    _coupled_val(coupledGenericValue<is_ad>("coupled"))
{
}

template <bool is_ad>
Real
CoupledAuxTempl<is_ad>::computeValue()
{
  if (_operator == "+")
    return MetaPhysicL::raw_value(_coupled_val[_qp]) + _value;
  else if (_operator == "-")
    return MetaPhysicL::raw_value(_coupled_val[_qp]) - _value;
  else if (_operator == "*")
    return MetaPhysicL::raw_value(_coupled_val[_qp]) * _value;
  else if (_operator == "/")
  // We are going to do division for this operation
  // This is useful for testing evaluation order
  // when we attempt to divide by zero!
  {
    if (_coupled_val[_qp] == 0)
      mooseError("Floating point exception in coupled_value");

    return _value / MetaPhysicL::raw_value(_coupled_val[_qp]);
  }

  // Won't reach this statement
  return 0;
}

template class CoupledAuxTempl<false>;
template class CoupledAuxTempl<true>;
