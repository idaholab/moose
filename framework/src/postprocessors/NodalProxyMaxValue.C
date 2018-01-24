//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalProxyMaxValue.h"
#include "MooseMesh.h"
#include "SubProblem.h"
// libMesh
#include "libmesh/boundary_info.h"

template <>
InputParameters
validParams<NodalProxyMaxValue>()
{
  InputParameters params = validParams<NodalVariablePostprocessor>();
  return params;
}

NodalProxyMaxValue::NodalProxyMaxValue(const InputParameters & parameters)
  : NodalVariablePostprocessor(parameters), _value(-std::numeric_limits<Real>::max())
{
}

void
NodalProxyMaxValue::initialize()
{
  _value = -std::numeric_limits<Real>::max();
}

Real
NodalProxyMaxValue::computeValue()
{
  return _u[_qp];
}

void
NodalProxyMaxValue::execute()
{
  Real val = computeValue();

  if (val > _value)
  {
    _value = val;
    _node_id = _current_node->id();
  }
}

Real
NodalProxyMaxValue::getValue()
{
  gatherProxyValueMax(_value, _node_id);
  return _node_id;
}

void
NodalProxyMaxValue::threadJoin(const UserObject & y)
{
  const NodalProxyMaxValue & pps = static_cast<const NodalProxyMaxValue &>(y);
  if (pps._value > _value)
  {
    _value = pps._value;
    _node_id = pps._node_id;
  }
}
