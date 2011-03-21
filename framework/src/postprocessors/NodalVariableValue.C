#include "NodalVariableValue.h"
#include "Mesh.h"
#include "SubProblem.h"

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
    _mesh(_problem.mesh()),
    _var_name(parameters.get<std::string>("variable")),
    _node(_mesh.node(parameters.get<unsigned int>("nodeid")))
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

  // FIXME: PPS: variable nodal value
//  if(_node.processor_id() == libMesh::processor_id())
//    value = _problem.getVariableNodalValue(_node, _var_name);

  gatherSum(value);

  return value;
}
