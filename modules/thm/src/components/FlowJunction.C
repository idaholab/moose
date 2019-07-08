#include "FlowJunction.h"
#include "GeometricalFlowComponent.h"
#include "THMMesh.h"

template <>
InputParameters
validParams<FlowJunction>()
{
  InputParameters params = validParams<FlowConnection>();
  params.addPrivateParam<std::string>("component_type", "flow_junction");
  params.addDeprecatedParam<std::vector<BoundaryName>>(
      "inputs",
      "Inputs of this junction",
      "Use 'connections' parameter instead. Put 'inputs' and 'outputs' in it.");
  params.addDeprecatedParam<std::vector<BoundaryName>>(
      "outputs",
      "Outputs of this junction",
      "Use 'connections' parameter instead. Put 'inputs' and 'outputs' in it.");
  params.addRequiredParam<std::vector<BoundaryName>>("connections", "Junction connections");
  return params;
}

FlowJunction::FlowJunction(const InputParameters & params) : FlowConnection(params)
{
  const std::vector<BoundaryName> & connections =
      getParam<std::vector<BoundaryName>>("connections");
  for (const auto & connection_string : connections)
    addConnection(connection_string);
}

void
FlowJunction::setupMesh()
{
  FlowConnection::setupMesh();

  const BoundaryID boundary_id = _mesh.getNextBoundaryId();

  // name the nodeset/sideset corresponding to the nodes of all connected flow channel ends
  _mesh.setBoundaryName(boundary_id, name());

  for (const auto & connection : getConnections())
  {
    const std::string & comp_name = connection._geometrical_component_name;

    if (hasComponentByName<GeometricalFlowComponent>(comp_name))
    {
      const GeometricalFlowComponent & gc = getComponentByName<GeometricalFlowComponent>(comp_name);
      for (auto && conn : gc.getConnections(connection._end_type))
        // add connection's node to nodeset of all nodes connected to this zero-D component
        _mesh.getMesh().boundary_info->add_node(conn._node, boundary_id);
    }
  }
}
