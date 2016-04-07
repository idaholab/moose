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

template<>
InputParameters validParams<NodalVariableValue>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<VariableName>("variable", "The variable to be monitored");
  params.addRequiredParam<unsigned int>("nodeid", "The ID of the node where we monitor");
  params.addParam<Real>("scale_factor", 1, "A scale factor to be applied to the variable");
  return params;
}

NodalVariableValue::NodalVariableValue(const InputParameters & parameters) :
    GeneralPostprocessor(parameters),
    _var_name(parameters.get<VariableName>("variable")),
    _node_ptr(_mesh.getMesh().query_node_ptr(getParam<unsigned int>("nodeid"))),
    _scale_factor(getParam<Real>("scale_factor"))
{
  // This class only works with SerialMesh, since it relies on a
  // specific node numbering that we can't guarantee with ParallelMesh
  _mesh.errorIfParallelDistribution("NodalVariableValue");

  if (_node_ptr == NULL)
    mooseError("Node #" << getParam<unsigned int>("nodeid") << " specified in '" << name() << "' not found in the mesh!");
}

Real
NodalVariableValue::getValue()
{
  Real value = 0;

  if (_node_ptr->processor_id() == processor_id())
    value = _subproblem.getVariable(0, _var_name).getNodalValue(*_node_ptr);

  gatherSum(value);

  return _scale_factor * value;
}

