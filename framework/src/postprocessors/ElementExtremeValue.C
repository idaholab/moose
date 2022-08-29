//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementExtremeValue.h"

#include <algorithm>
#include <limits>

registerMooseObject("MooseApp", ElementExtremeValue);

InputParameters
ElementExtremeValue::validParams()
{
  // Define the min/max enumeration
  MooseEnum type_options("max=0 min=1", "max");

  // Define the parameters
  InputParameters params = ElementVariablePostprocessor::validParams();

  params.addParam<MooseEnum>("value_type",
                             type_options,
                             "Type of extreme value to return. 'max' "
                             "returns the maximum value. 'min' returns "
                             "the minimum value.");

  params.addCoupledVar("proxy_variable",
                       "The name of the variable to use to identify the location at which "
                       "the variable value should be taken; if not provided, this defaults "
                       "to the 'variable'.");

  params.addClassDescription(
      "Finds either the min or max elemental value of a variable over the domain.");

  return params;
}

ElementExtremeValue::ElementExtremeValue(const InputParameters & parameters)
  : ElementVariablePostprocessor(parameters),
    _type((ExtremeType)(int)parameters.get<MooseEnum>("value_type")),
    _proxy_variable(isParamValid("proxy_variable") ? coupledValue("proxy_variable") : _u)
{
}

void
ElementExtremeValue::initialize()
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
ElementExtremeValue::computeQpValue()
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
ElementExtremeValue::getValue()
{
  return _value;
}

void
ElementExtremeValue::finalize()
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
ElementExtremeValue::threadJoin(const UserObject & y)
{
  const ElementExtremeValue & pps = static_cast<const ElementExtremeValue &>(y);

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
