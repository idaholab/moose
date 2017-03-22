/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "ElementExtremeValue.h"

#include <algorithm>
#include <limits>

template <>
InputParameters
validParams<ElementExtremeValue>()
{
  // Define the min/max enumeration
  MooseEnum type_options("max=0 min=1", "max");

  // Define the parameters
  InputParameters params = validParams<ElementVariablePostprocessor>();

  params.addParam<MooseEnum>("value_type",
                             type_options,
                             "Type of extreme value to return. 'max' "
                             "returns the maximum value. 'min' returns "
                             "the minimum value.");

  return params;
}

ElementExtremeValue::ElementExtremeValue(const InputParameters & parameters)
  : ElementVariablePostprocessor(parameters),
    _type((ExtremeType)(int)parameters.get<MooseEnum>("value_type")),
    _value(_type == 0 ? -std::numeric_limits<Real>::max() : std::numeric_limits<Real>::max())
{
}

void
ElementExtremeValue::initialize()
{
  switch (_type)
  {
    case MAX:
      _value = -std::numeric_limits<Real>::max(); // start w/ the min
      break;

    case MIN:
      _value = std::numeric_limits<Real>::max(); // start w/ the max
      break;
  }
}

void
ElementExtremeValue::computeQpValue()
{
  switch (_type)
  {
    case MAX:
      _value = std::max(_value, _u[_qp]);
      break;

    case MIN:
      _value = std::min(_value, _u[_qp]);
      break;
  }
}

Real
ElementExtremeValue::getValue()
{
  switch (_type)
  {
    case MAX:
      gatherMax(_value);
      break;
    case MIN:
      gatherMin(_value);
      break;
  }

  return _value;
}

void
ElementExtremeValue::threadJoin(const UserObject & y)
{
  const ElementExtremeValue & pps = static_cast<const ElementExtremeValue &>(y);

  switch (_type)
  {
    case MAX:
      _value = std::max(_value, pps._value);
      break;
    case MIN:
      _value = std::min(_value, pps._value);
      break;
  }
}
