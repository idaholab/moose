#include "FlowJunction.h"
#include "GeometricalFlowComponent.h"
#include "RELAP7App.h"
#include "FluidProperties.h"

template <>
InputParameters
validParams<FlowJunction>()
{
  InputParameters params = validParams<FlowConnection>();
  params.addPrivateParam<std::string>("component_type", "flow_junction");
  params.addRequiredParam<std::vector<std::string>>("inputs", "Inputs of this junction");
  params.addRequiredParam<std::vector<std::string>>("outputs", "Outputs of this junction");
  return params;
}

FlowJunction::FlowJunction(const InputParameters & params) : FlowConnection(params)
{
  const std::vector<std::string> & inputs = getParam<std::vector<std::string>>("inputs");
  const std::vector<std::string> & outputs = getParam<std::vector<std::string>>("outputs");
  for (const auto & connection_string : inputs)
    addConnection(connection_string);
  for (const auto & connection_string : outputs)
    addConnection(connection_string);
}

void
FlowJunction::setupMesh()
{
  FlowConnection::setupMesh();

  const unsigned int boundary_id = getNextBoundaryId();

  // name the nodeset/sideset corresponding to the nodes of all connected pipe ends
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

std::string
FlowJunction::createBoundaryName(const std::string & comp_name) const
{
  return name() + ":" + comp_name;
}
