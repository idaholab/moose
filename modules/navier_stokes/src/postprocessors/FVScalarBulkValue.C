//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVScalarBulkValue.h"

registerMooseObject("NavierStokesApp", FVScalarBulkValue);

InputParameters
FVScalarBulkValue::validParams()
{
  InputParameters params = ElementPostprocessor::validParams();
  params.addClassDescription(
      "Average a variable value using a weight mask given by a material property.");
  params.addRequiredParam<MooseFunctorName>("scalar", "Variable to perform weighted average on.");
  params.addRequiredParam<MooseFunctorName>("velocity",
                                                "Wei.");
  return params;
}

FVScalarBulkValue::FVScalarBulkValue(const InputParameters & parameters)
  : ElementPostprocessor(parameters),
   _scalar(getFunctor<Real>("scalar")),
   _u(getFunctor<Real>("velocity"))
{
}

void
FVScalarBulkValue::initialize()
{
  _var_integral = 0.0;
  _weight_integral = 0.0;
}

void
FVScalarBulkValue::execute()
{
    const auto state = determineState();
    const auto elem_arg = Moose::ElemArg(
            {_current_elem});	 
    _var_integral += _scalar(elem_arg, state) * _u(elem_arg, state);
    _weight_integral += _u(elem_arg, state);
}

void
FVScalarBulkValue::finalize()
{
  gatherSum(_var_integral);
  gatherSum(_weight_integral);
}

PostprocessorValue
FVScalarBulkValue::getValue() const
{
  if (_weight_integral == 0.0)
    return 0.0;

  return _var_integral / _weight_integral;
}

void
FVScalarBulkValue::threadJoin(const UserObject & y)
{
  const auto & pp = static_cast<const FVScalarBulkValue &>(y);
  _var_integral += pp._var_integral;
  _weight_integral += pp._weight_integral;
}
