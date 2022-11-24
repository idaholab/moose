//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalMaxValue.h"

#include <algorithm>
#include <limits>

registerMooseObjectReplaced("MooseApp", NodalMaxValue, "09/27/2021 00:00", NodalExtremeValue);

InputParameters
NodalMaxValue::validParams()
{
  InputParameters params = NodalVariablePostprocessor::validParams();
  params.addClassDescription("Computes the maximum (over all the nodal values) of a variable.");
  return params;
}

NodalMaxValue::NodalMaxValue(const InputParameters & parameters)
  : NodalVariablePostprocessor(parameters), _value(-std::numeric_limits<Real>::max())
{
}

void
NodalMaxValue::initialize()
{
  _value = -std::numeric_limits<Real>::max();
}

void
NodalMaxValue::execute()
{
  _value = std::max(_value, _u[_qp]);
}

Real
NodalMaxValue::getValue()
{
  return _value;
}
void
NodalMaxValue::finalize()
{
  gatherMax(_value);
}
void
NodalMaxValue::threadJoin(const UserObject & y)
{
  const NodalMaxValue & pps = static_cast<const NodalMaxValue &>(y);
  _value = std::max(_value, pps._value);
}
