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
#include "libmesh/boundary_info.h"

template<>
InputParameters validParams<NodalVariableValue>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<VariableName>("variable", "The variable to be monitored");
  params.addRequiredParam<unsigned int>("nodeid", "The ID of the node where we monitor");
  return params;
}

NodalVariableValue::NodalVariableValue(const std::string & name, InputParameters parameters) :
    GeneralPostprocessor(name, parameters),
    _mesh(_subproblem.mesh()),
    _var_name(parameters.get<VariableName>("variable")),
    _node_ptr(_mesh.getMesh().query_node_ptr(getParam<unsigned int>("nodeid")))
{
  if (_node_ptr == NULL)
    mooseError("Node #" << getParam<unsigned int>("nodeid") << " specified in '" << name << "' not found in the mesh!");
}

Real
NodalVariableValue::getValue()
{
  Real value = 0;

  if(_node_ptr->processor_id() == libMesh::processor_id())
    value = _subproblem.getVariable(_tid, _var_name).getNodalValue(*_node_ptr);

  gatherSum(value);

  return value;
}
