//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalExtremeValue.h"

#include <algorithm>
#include <limits>

registerMooseObject("MooseApp", NodalExtremeValue);

InputParameters
NodalExtremeValue::validParams()
{
  // Define the min/max enumeration
  MooseEnum type_options("max=0 min=1", "max");

  // Define the parameters
  InputParameters params = NodalVariablePostprocessor::validParams();
  params.addClassDescription("Reports the minimum or maximum value of a variable.");

  params.addParam<MooseEnum>("value_type",
                             type_options,
                             "Type of extreme value to return. 'max' "
                             "returns the maximum value. 'min' returns "
                             "the minimum value.");

  params.addCoupledVar("proxy_variable",
                       "The name of the variable to use to identify the location at which "
                       "the variable value should be taken; if not provided, this defaults "
                       "to the 'variable'.");
  return params;
}

NodalExtremeValue::NodalExtremeValue(const InputParameters & parameters)
  : NodalVariablePostprocessor(parameters),
    _type((ExtremeType)(int)parameters.get<MooseEnum>("value_type")),
    _proxy_variable(isParamValid("proxy_variable") ? coupledValue("proxy_variable") : _u)
{
}

void
NodalExtremeValue::initialize()
{
  switch (_type)
  {
    case MAX:
      _proxy_value = -std::numeric_limits<Real>::max(); // start w/ the min
      _value = -std::numeric_limits<Real>::max();
      break;

    case MIN:
      _proxy_value = std::numeric_limits<Real>::max(); // start w/ the max
      _value = std::numeric_limits<Real>::max();
      break;
  }
}

void
NodalExtremeValue::execute()
{
  switch (_type)
  {
    case MAX:
      if (_proxy_variable[_qp] > _proxy_value)
      {
        _proxy_value = _proxy_variable[_qp];
        _value = _u[_qp];
      }
      break;

    case MIN:
      if (_proxy_variable[_qp] < _proxy_value)
      {
        _proxy_value = _proxy_variable[_qp];
        _value = _u[_qp];
      }
      break;
  }
}

Real
NodalExtremeValue::getValue()
{
  return _value;
}

void
NodalExtremeValue::finalize()
{
  switch (_type)
  {
    case MAX:
      gatherProxyValueMax(_proxy_value, _value);
      break;
    case MIN:
      gatherProxyValueMin(_proxy_value, _value);
      break;
  }
}

void
NodalExtremeValue::threadJoin(const UserObject & y)
{
  const NodalExtremeValue & pps = static_cast<const NodalExtremeValue &>(y);

  switch (_type)
  {
    case MAX:
      if (pps._proxy_value > _proxy_value)
      {
        _proxy_value = pps._proxy_value;
        _value = pps._value;
      }
      break;
    case MIN:
      if (pps._proxy_value < _proxy_value)
      {
        _proxy_value = pps._proxy_value;
        _value = pps._value;
      }
      break;
  }
}
