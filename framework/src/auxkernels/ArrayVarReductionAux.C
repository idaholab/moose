//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  params.addParam<RealEigenVector>(
      "weights",
      "Relevant when 'value_type' is sum or average. When computing an average, these are the "
      "weights for a weighted average and when computing a sum, these weight scale each variable "
      "component in the summation.");
  params.addClassDescription("Takes an array variable and performs a reduction operation on it "
                             "(max, min, sum, average) and stores as a standard variable.");
  return params;
}

ArrayVarReductionAux::ArrayVarReductionAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _array_variable(coupledArrayValue("array_variable")),
    _value_type(getParam<MooseEnum>("value_type")),
    _weights(isParamValid("weights")
                 ? getParam<RealEigenVector>("weights")
                 : RealEigenVector::Ones(getArrayVar("array_variable", 0)->count()))
{
  const auto array_comps = getArrayVar("array_variable", 0)->count();
  if (_weights.size() != array_comps)
    paramError(
        "weights",
        "The number of values provided is " + std::to_string(_weights.size()) +
            " but the number of components for the variable provided by 'array_variable' is " +
            std::to_string(array_comps));
  if (isParamValid("weights") && !(_value_type == "average" || _value_type == "sum"))
    paramError("weights", "Is only meant to be be used when 'value_type' is average or sum");
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
      return _weights.cwiseProduct(_array_variable[_qp]).sum();

    case 3:
      return _weights.cwiseProduct(_array_variable[_qp]).sum() / _weights.sum();
  }

  return 0.0;
}
