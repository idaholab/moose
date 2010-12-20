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

#include "AverageNodalVariableValue.h"
#include "MooseSystem.h"

// libMesh
#include "boundary_info.h"

template<>
InputParameters validParams<AverageNodalVariableValue>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<std::string>("variable", "The variable to be monitored");
  params.addRequiredParam<unsigned int>("nodeset", "The ID of the node where we monitor");
  return params;
}

AverageNodalVariableValue::AverageNodalVariableValue(const std::string & name, InputParameters parameters) :
  GeneralPostprocessor(name, parameters),
  _mesh(*_moose_system.getMesh()),
  _var_name(getParam<std::string>("variable")),
  _nodesetid(getParam<unsigned int>("nodeset"))
{
  std::vector<unsigned int> nodes;
  std::vector<short int> ids;
  _mesh.boundary_info->build_node_list(nodes, ids);

  for (unsigned int i = 0; i < nodes.size(); i++)
  {
    if (ids[i] == _nodesetid)
      _node_ids.push_back(nodes[i]);
  }
}

Real
AverageNodalVariableValue::getValue()
{
  Real avg = 0;
  int n = _node_ids.size();
  for (int i = 0; i < n; i++)
  {
    if(_mesh.node(_node_ids[i]).processor_id() == libMesh::processor_id())
      avg += _moose_system.getVariableNodalValue(_mesh.node(_node_ids[i]), _var_name);
  }

  gatherSum(avg);  

  return avg / n;
}
