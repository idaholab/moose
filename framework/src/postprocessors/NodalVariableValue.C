//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalVariableValue.h"

// MOOSE includes
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "SubProblem.h"

#include "libmesh/node.h"

registerMooseObject("MooseApp", NodalVariableValue);

InputParameters
NodalVariableValue::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<VariableName>("variable", "The variable to be monitored");
  params.addRequiredParam<unsigned int>("nodeid", "The ID of the node where we monitor");
  params.addParam<Real>("scale_factor", 1, "A scale factor to be applied to the variable");
  params.addClassDescription("Outputs values of a nodal variable at a particular location");
  return params;
}

NodalVariableValue::NodalVariableValue(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _mesh(_subproblem.mesh()),
    _var_name(parameters.get<VariableName>("variable")),
    _node_ptr(nullptr),
    _scale_factor(getParam<Real>("scale_factor")),
    _value(0)
{
  // This class may be too dangerous to use if renumbering is enabled,
  // as the nodeid parameter obviously depends on a particular
  // numbering.
  if (_mesh.getMesh().allow_renumbering())
    mooseError("NodalVariableValue should only be used when node renumbering is disabled.");
}

void
NodalVariableValue::initialSetup()
{
  _node_ptr = _mesh.getMesh().query_node_ptr(getParam<unsigned int>("nodeid"));
  bool found_node_ptr = _node_ptr;
  _communicator.max(found_node_ptr);

  if (!found_node_ptr)
    mooseError("Node #",
               getParam<unsigned int>("nodeid"),
               " specified in '",
               name(),
               "' not found in the mesh!");
}

void
NodalVariableValue::execute()
{
  _value = 0;

  if (_node_ptr && _node_ptr->processor_id() == processor_id())
    _value = _subproblem.getStandardVariable(_tid, _var_name).getNodalValue(*_node_ptr);
}

Real
NodalVariableValue::getValue()
{

  return _scale_factor * _value;
}

void
NodalVariableValue::finalize()
{
  gatherSum(_value);
}
