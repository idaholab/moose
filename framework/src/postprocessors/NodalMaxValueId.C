//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalMaxValueId.h"
#include "MooseMesh.h"
#include "SubProblem.h"
// libMesh
#include "libmesh/boundary_info.h"

registerMooseObject("MooseApp", NodalMaxValueId);
registerMooseObjectRenamed("MooseApp", NodalProxyMaxValue, "04/01/2022 00:00", NodalMaxValueId);

InputParameters
NodalMaxValueId::validParams()
{
  InputParameters params = NodalVariablePostprocessor::validParams();
  params.addClassDescription(
      "Finds the node id with the maximum nodal value across all postprocessors.");
  return params;
}

NodalMaxValueId::NodalMaxValueId(const InputParameters & parameters)
  : NodalVariablePostprocessor(parameters), _value(-std::numeric_limits<Real>::max())
{
}

void
NodalMaxValueId::initialize()
{
  _value = -std::numeric_limits<Real>::max();
}

Real
NodalMaxValueId::computeValue()
{
  return _u[_qp];
}

void
NodalMaxValueId::execute()
{
  Real val = computeValue();

  if (val > _value)
  {
    _value = val;
    _node_id = _current_node->id();
  }
}

Real
NodalMaxValueId::getValue()
{
  return _node_id;
}
void
NodalMaxValueId::finalize()
{
  gatherProxyValueMax(_value, _node_id);
}

void
NodalMaxValueId::threadJoin(const UserObject & y)
{
  const NodalMaxValueId & pps = static_cast<const NodalMaxValueId &>(y);
  if (pps._value > _value)
  {
    _value = pps._value;
    _node_id = pps._node_id;
  }
}
