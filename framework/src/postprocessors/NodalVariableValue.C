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
#include "MooseSystem.h"

// libMesh
#include "boundary_info.h"

template<>
InputParameters validParams<NodalVariableValue>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<std::string>("variable", "The variable to be monitored");
  params.addRequiredParam<unsigned int>("nodeid", "The ID of the node where we monitor");
  return params;
}

NodalVariableValue::NodalVariableValue(const std::string & name, InputParameters parameters) :
  GeneralPostprocessor(name, parameters),
  _mesh(*_moose_system.getMesh()),
  _var_name(getParam<std::string>("variable")),
  _node(_mesh.node(getParam<unsigned int>("nodeid")))
{
/*
  std::vector<unsigned int> nodes;
  std::vector<short int> ids;
  _mesh.boundary_info->build_node_list(nodes, ids);

  bool found = false;
  int i;
  for (i = 0; i < nodes.size(); i++)
  {
//    if (nodes[i] == _nodeid)
//    {
//      found = true;
//      break;
//    }
    std::cout << "n=" << nodes[i] << ", " << ids[i] << std::endl;
  }

  if (!found)
    mooseError("Specified nodeid was not found in any nodeset");

//  Node & node = _mesh.node(id);
*/
}

Real
NodalVariableValue::getValue()
{
  Real value = 0;

  if(_node.processor_id() == libMesh::processor_id())
    value = _moose_system.getVariableNodalValue(_node, _var_name);

  gatherSum(value);

  return value;
}
