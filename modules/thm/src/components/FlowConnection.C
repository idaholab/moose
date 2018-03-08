#include "FlowConnection.h"
#include "GeometricalFlowComponent.h"

const std::map<std::string, FlowConnection::EEndType> FlowConnection::_end_type_to_enum{
    {"IN", IN}, {"OUT", OUT}};

template <>
FlowConnection::EEndType
RELAP7::stringToEnum(const std::string & s)
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
      const GeometricalFlowComponent & gc = getComponentByName<GeometricalFlowComponent>(comp_name);
      for (auto && conn : gc.getConnections(connection._end_type))
      {
        const BoundaryName boundary_name = createBoundaryName(comp_name);

        // get info from the connection
        _positions.push_back(conn._position);
        _nodes.push_back(conn._node->id());
        _normals.push_back(conn._normal);
        _boundary_ids.push_back(conn._boundary_id);
        _boundary_names.push_back(boundary_name);
        _connection_strings.push_back(connection._string);

        // name the nodeset/sideset corresponding to the node of the connected pipe end
        _mesh.setBoundaryName(conn._boundary_id, boundary_name);

        // add connection's node to nodeset of all boundary nodes
        _mesh.getMesh().boundary_info->add_node(conn._node, RELAP7::bnd_nodeset_id);
      }
    }
  }
}

void
FlowConnection::init()
{
  Component::init();

  std::vector<UserObjectName> fp_names;
  std::vector<RELAP7::FlowModelID> flow_model_ids;
  std::vector<bool> implicit_rdg_flags;
  std::vector<FlowModel::ESpatialDiscretizationType> spatial_discretizations;
  for (const auto & connection : _connections)
  {
    const std::string comp_name = connection._geometrical_component_name;
    const GeometricalFlowComponent & comp =
        _sim.getComponentByName<GeometricalFlowComponent>(comp_name);

    // add to list of subdomain IDs
    const std::vector<unsigned int> & ids = comp.getSubdomainIds();
    _connected_subdomain_ids.insert(_connected_subdomain_ids.end(), ids.begin(), ids.end());

    fp_names.push_back(comp.getFluidPropertiesName());
    flow_model_ids.push_back(comp.getFlowModelID());
    _rdg_flux_names.push_back(comp.getRDGFluxUserObjectName());
    implicit_rdg_flags.push_back(comp.getImplicitRDGFlag());
    spatial_discretizations.push_back(comp.getSpatialDiscretizationType());
  }

  checkAllConnectionsHaveSame<UserObjectName>(fp_names, "fluid properties object");
  _fp_name = fp_names[0];

  checkAllConnectionsHaveSame<RELAP7::FlowModelID>(flow_model_ids, "flow model ID");
  _flow_model_id = flow_model_ids[0];

  checkAllConnectionsHaveSame<bool>(implicit_rdg_flags, "implicit rDG flag");
  _implicit_rdg = implicit_rdg_flags[0];

  checkAllConnectionsHaveSame<FlowModel::ESpatialDiscretizationType>(spatial_discretizations,
                                                                     "spatial discretization");
  _spatial_discretization = spatial_discretizations[0];
}

void
FlowConnection::check()
{
  Component::check();

  for (const auto & comp_name : _connected_component_names)
    checkComponentOfTypeExistsByName<GeometricalFlowComponent>(comp_name);
}

void
FlowConnection::addConnection(const std::string & connection_string)
{
  // check for correct format
  const size_t bpos = connection_string.find('(');
  const size_t epos = connection_string.find(')', bpos);
  if ((bpos == std::string::npos) || (epos == std::string::npos))
    mooseError("Incorrect connection format");

  // extract component name
  const std::string connected_component_name = connection_string.substr(0, bpos);

  // extract end type
  const std::string end = connection_string.substr(bpos + 1, epos - bpos - 1);
  const EEndType end_type = RELAP7::stringToEnum<EEndType>(end);

  // store connection data
  _connections.push_back(Connection(connection_string, connected_component_name, end_type));
  _connected_component_names.push_back(connected_component_name);

  // Add dependency because the connected component's setupMesh() must be called
  // before this component's setupMesh().
  addDependency(connected_component_name);
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
