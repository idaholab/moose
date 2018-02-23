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

FlowConnection::FlowConnection(const InputParameters & params) : Component(params) {}

void
FlowConnection::init()
{
  Component::init();

  // create list of subdomain IDs
  for (const auto & comp_name : _connected_component_names)
  {
    const GeometricalFlowComponent & comp =
        _sim.getComponentByName<GeometricalFlowComponent>(comp_name);
    const std::vector<unsigned int> & ids = comp.getSubdomainIds();
    _connected_subdomain_ids.insert(_connected_subdomain_ids.end(), ids.begin(), ids.end());
  }
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
