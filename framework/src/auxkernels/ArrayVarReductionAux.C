//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayVarReductionAux.h"

registerMooseObject("MooseApp", ArrayVarReductionAux);

InputParameters
ArrayVarReductionAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredCoupledVar("array_variable", "Array variable to process.");
  params.addParam<MooseEnum>(
      "value_type",
      MooseEnum("max=0 min=1 sum=2 average=3", "sum"),
      "Type of reduction operation. Options are max, min, sum, and average.");
  params.addClassDescription("Takes an array variable and performs a reduction operation on it "
                             "(max, min, sum, average) and stores as a standard variable.");
  return params;
}

ArrayVarReductionAux::ArrayVarReductionAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _nentries(getArrayVar("array_variable", 0)->count()),
    _array_variable(coupledArrayValue("array_variable")),
    _value_type(getParam<MooseEnum>("value_type"))
{
}

Real
ArrayVarReductionAux::computeValue()
{
  Real r = 0;
  switch (_value_type)
  {
    case 0:
      r = std::numeric_limits<Real>::lowest();
      for (unsigned int t = 0; t < _nentries; ++t)
        if (_array_variable[_qp](t) > r)
          r = _array_variable[_qp](t);
      break;

    case 1:
      r = std::numeric_limits<Real>::max();
      for (unsigned int t = 0; t < _nentries; ++t)
        if (_array_variable[_qp](t) < r)
          r = _array_variable[_qp](t);
      break;

    case 2:
      r = 0;
      for (unsigned int t = 0; t < _nentries; ++t)
        r += _array_variable[_qp](t);
      break;

    case 3:
      r = 0;
      for (unsigned int t = 0; t < _nentries; ++t)
        r += _array_variable[_qp](t) / _nentries;
      return r;
  }
  return r;
}
