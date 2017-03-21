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

#include "NodalVariableValue.h"
#include "MooseMesh.h"
#include "SubProblem.h"

// libMesh
#include "libmesh/node.h"

template <>
InputParameters
validParams<NodalVariableValue>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<VariableName>("variable", "The variable to be monitored");
  params.addRequiredParam<unsigned int>("nodeid", "The ID of the node where we monitor");
  params.addParam<Real>("scale_factor", 1, "A scale factor to be applied to the variable");
  return params;
}

NodalVariableValue::NodalVariableValue(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _mesh(_subproblem.mesh()),
    _var_name(parameters.get<VariableName>("variable")),
    _node_ptr(_mesh.getMesh().query_node_ptr(getParam<unsigned int>("nodeid"))),
    _scale_factor(getParam<Real>("scale_factor"))
{
  // This class may be too dangerous to use if renumbering is enabled,
  // as the nodeid parameter obviously depends on a particular
  // numbering.
  if (_mesh.getMesh().allow_renumbering())
    mooseError("NodalVariableValue should only be used when node renumbering is disabled.");

  bool found_node_ptr = _node_ptr;
  _communicator.max(found_node_ptr);

  if (!found_node_ptr)
    mooseError("Node #",
               getParam<unsigned int>("nodeid"),
               " specified in '",
               name(),
               "' not found in the mesh!");
}

Real
NodalVariableValue::getValue()
{
  Real value = 0;

  if (_node_ptr && _node_ptr->processor_id() == processor_id())
    value = _subproblem.getVariable(_tid, _var_name).getNodalValue(*_node_ptr);

  gatherSum(value);

  return _scale_factor * value;
}
