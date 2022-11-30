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
    _array_variable(coupledArrayValue("array_variable")),
    _value_type(getParam<MooseEnum>("value_type"))
{
}

Real
ArrayVarReductionAux::computeValue()
{
  switch (_value_type)
  {
    case 0:
      return _array_variable[_qp].maxCoeff();

    case 1:
      return _array_variable[_qp].minCoeff();

    case 2:
      return _array_variable[_qp].sum();

    case 3:
      return _array_variable[_qp].mean();
  }

  return 0.0;
}
