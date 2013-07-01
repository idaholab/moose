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

#include "NodalExtremeValue.h"

#include <algorithm>
#include <limits>

template<>
InputParameters validParams<NodalExtremeValue>()
{
  // Define the min/max enumeration
  MooseEnum type_options("max=0, min=1", "max");

  // Define the parameters
  InputParameters params = validParams<NodalVariablePostprocessor>();
  params.addParam<MooseEnum>("value_type", type_options, "Type of extreme value to return. 'max' returns the maximum value. 'min' returns the minimu value.");
  return params;
}

NodalExtremeValue::NodalExtremeValue(const std::string & name, InputParameters parameters) :
  NodalVariablePostprocessor(name, parameters),
  _type(parameters.get<MooseEnum>("value_type")),
  _value(_type == 0 ? -std::numeric_limits<Real>::max() : std::numeric_limits<Real>::max())
{}

void
NodalExtremeValue::initialize()
{
  switch (_type)
  {
    case 0:
      _value = -std::numeric_limits<Real>::max(); // start w/ the min
      break;

    case 1:
      _value = std::numeric_limits<Real>::max(); // start w/ the max
      break;
  }
}

void
NodalExtremeValue::execute()
{
  switch (_type)
  {
    case 0:
      _value = std::max(_value, _u[_qp]);
      break;

    case 1:
      _value = std::min(_value, _u[_qp]);
      break;
  }
}

Real
NodalExtremeValue::getValue()
{
  switch (_type)
  {
    case 0:
      gatherMax(_value);
      break;
    case 1:
      gatherMin(_value);
      break;
  }

  return _value;
}

void
NodalExtremeValue::threadJoin(const UserObject & y)
{
  const NodalExtremeValue & pps = static_cast<const NodalExtremeValue &>(y);

  switch (_type)
  {
    case 0:
      _value = std::max(_value, pps._value);
      break;
    case 1:
      _value = std::min(_value, pps._value);
      break;
  }
}
