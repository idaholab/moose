#include "FlowConnection.h"
#include "GeometricalFlowComponent.h"
#include "PipeBase.h"
#include "FlowModelTwoPhase.h"
#include "RELAP7Mesh.h"

const std::map<std::string, FlowConnection::EEndType> FlowConnection::_end_type_to_enum{
    {"IN", IN}, {"OUT", OUT}};

template <>
FlowConnection::EEndType
THM::stringToEnum(const std::string & s)
{
  return stringToEnum<FlowConnection::EEndType>(s, FlowConnection::_end_type_to_enum);
}

template <>
InputParameters
validParams<FlowConnection>()
{
  InputParameters params = validParams<Component>();
  return params;
}

FlowConnection::FlowConnection(const InputParameters & params)
  : Component(params), _flow_model_id(RELAP7::FM_INVALID)
{
}

void
FlowConnection::setupMesh()
{
  for (const auto & connection : _connections)
  {
    const std::string & comp_name = connection._geometrical_component_name;

    if (hasComponentByName<GeometricalFlowComponent>(comp_name))
    {
      _boundary_names.push_back(connection._boundary_name);

      const GeometricalFlowComponent & gc = getComponentByName<GeometricalFlowComponent>(comp_name);
      for (auto && conn : gc.getConnections(connection._end_type))
      {
        // get info from the connection
        _positions.push_back(conn._position);
        _nodes.push_back(conn._node->id());
        _normals.push_back(conn._normal);
        _boundary_ids.push_back(conn._boundary_id);

        // add connection's node to nodeset of all boundary nodes
        _mesh.getMesh().boundary_info->add_node(conn._node, RELAP7::bnd_nodeset_id);
      }
    }
    else
      logError("Trying to connect to a component '",
               comp_name,
               "', but there is no such component in the simulation. Please check your spelling.");
  }
}

void
FlowConnection::init()
{
  Component::init();

  if (_connections.size() > 0)
  {
    std::vector<UserObjectName> fp_names;
    std::vector<RELAP7::FlowModelID> flow_model_ids;
    for (const auto & connection : _connections)
    {
      const std::string comp_name = connection._geometrical_component_name;
      if (hasComponentByName<GeometricalFlowComponent>(comp_name))
      {
        const GeometricalFlowComponent & comp =
            _sim.getComponentByName<GeometricalFlowComponent>(comp_name);

        // add to list of subdomain IDs
        const std::vector<unsigned int> & ids = comp.getSubdomainIds();
        _connected_subdomain_ids.insert(_connected_subdomain_ids.end(), ids.begin(), ids.end());

        fp_names.push_back(comp.getFluidPropertiesName());
        flow_model_ids.push_back(comp.getFlowModelID());
        _A_linear_names.push_back(comp.getLinearAreaName());
        _numerical_flux_names.push_back(comp.getNumericalFluxUserObjectName());
        _rdg_int_var_uo_names.push_back(comp.getRDGInterfacialVariablesUserObjectName());
      }
    }

    if (fp_names.size() > 0)
    {
      checkAllConnectionsHaveSame<UserObjectName>(fp_names, "fluid properties object");
      _fp_name = fp_names[0];

      checkAllConnectionsHaveSame<RELAP7::FlowModelID>(flow_model_ids, "flow model ID");
      _flow_model_id = flow_model_ids[0];

      if (hasComponentByName<PipeBase>(_connections[0]._geometrical_component_name))
      {
        const PipeBase & pipe = getComponentByName<PipeBase>(_connections[0]._geometrical_component_name);
        if (_flow_model_id == RELAP7::FM_TWO_PHASE || _flow_model_id == RELAP7::FM_TWO_PHASE_NCG)
        {
          _flow_model = pipe.getFlowModel();
          auto flow_model_2phase = dynamic_cast<const FlowModelTwoPhase &>(*_flow_model);
          _phase_interaction = flow_model_2phase.getPhaseInteraction();
        }
      }
    }
  }
  else
    logError("The component is not connected.");
}

void
FlowConnection::check() const
{
  Component::check();

  for (const auto & comp_name : _connected_component_names)
    checkComponentOfTypeExistsByName<GeometricalFlowComponent>(comp_name);
}

void
FlowConnection::addConnection(const BoundaryName & boundary_name)
{
  const size_t oparenthesis_pos = boundary_name.find('(');
  if (oparenthesis_pos != std::string::npos)
  {
    logError("You are using the old connection format 'comp_name(end)'. Please update your input "
             "file to the new one 'comp_name:end'.");
  }
  else
  {
    const size_t colon_pos = boundary_name.rfind(':');
    // if it has a colon, assume 'component_name:end_type' format
    if (colon_pos != std::string::npos)
    {
      const std::string connected_component_name = boundary_name.substr(0, colon_pos);
      const std::string str_end =
          boundary_name.substr(colon_pos + 1, boundary_name.length() - colon_pos - 1);
      const EEndType end_type = THM::stringToEnum<EEndType>(str_end);

      _connections.push_back(Connection(boundary_name, connected_component_name, end_type));
      _connected_component_names.push_back(connected_component_name);

      // Add dependency because the connected component's setupMesh() must be called
      // before this component's setupMesh().
      addDependency(connected_component_name);
    }
    else
    {
      logError("Incorrect connection specified '",
               boundary_name,
               "'. Valid connection format is 'component_name:end_type'.");
    }
  }
}

void
FlowConnection::checkNumberOfConnections(const unsigned int & n_connections) const
{
  if (_connections.size() != n_connections)
    logError("The number of connections (", _connections.size(), ") must equal ", n_connections);
}

const std::vector<dof_id_type> &
FlowConnection::getNodeIDs() const
{
  checkSetupStatus(MESH_PREPARED);

  return _nodes;
}

const std::vector<BoundaryName> &
FlowConnection::getBoundaryNames() const
{
  checkSetupStatus(MESH_PREPARED);

  return _boundary_names;
}

const UserObjectName &
FlowConnection::getFluidPropertiesName() const
{
  checkSetupStatus(INITIALIZED_PRIMARY);

  return _fp_name;
}
