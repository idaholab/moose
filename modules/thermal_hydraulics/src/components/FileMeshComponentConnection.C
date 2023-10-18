//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FileMeshComponentConnection.h"
#include "FileMeshComponent.h"

const std::map<std::string, FileMeshComponentConnection::EEndType>
    FileMeshComponentConnection::_end_type_to_enum{{"IN", IN}, {"OUT", OUT}};

template <>
FileMeshComponentConnection::EEndType
THM::stringToEnum(const std::string & s)
{
  return stringToEnum<FileMeshComponentConnection::EEndType>(
      s, FileMeshComponentConnection::_end_type_to_enum);
}

InputParameters
FileMeshComponentConnection::validParams()
{
  InputParameters params = Component::validParams();
  return params;
}

FileMeshComponentConnection::FileMeshComponentConnection(const InputParameters & params)
  : Component(params)
{
}

void
FileMeshComponentConnection::setupMesh()
{
  // Copy the connection data from each specified component to this connection component
  for (const auto & connection : _connections)
  {
    const std::string & comp_name = connection._component_name;

    if (hasComponentByName<FileMeshComponent>(comp_name))
    {
      _boundary_names.push_back(connection._boundary_name);

      const FileMeshComponent & comp = getComponentByName<FileMeshComponent>(comp_name);
      for (auto && conn : comp.getConnections(connection._end_type))
      {
        // copy info from the connection
        _boundary_ids.push_back(conn._boundary_id);
      }
    }
    else
      logError("Trying to connect to a component '",
               comp_name,
               "', but there is no such component in the simulation. Please check your spelling.");
  }
}

void
FileMeshComponentConnection::init()
{
  Component::init();

  if (_connections.size() > 0)
  {
    std::vector<UserObjectName> fp_names;
    std::vector<THM::FlowModelID> flow_model_ids;
    for (const auto & connection : _connections)
    {
      const std::string comp_name = connection._component_name;
      if (hasComponentByName<FileMeshComponent>(comp_name))
      {
        const FileMeshComponent & comp =
            getTHMProblem().getComponentByName<FileMeshComponent>(comp_name);

        // add to list of subdomain names
        const std::vector<SubdomainName> & subdomain_names = comp.getSubdomainNames();
        _connected_subdomain_names.insert(
            _connected_subdomain_names.end(), subdomain_names.begin(), subdomain_names.end());
      }
    }
  }
  else
    logError("The component is not connected.");
}

void
FileMeshComponentConnection::check() const
{
  Component::check();

  for (const auto & comp_name : _connected_component_names)
    checkComponentOfTypeExistsByName<FileMeshComponent>(comp_name);
}

void
FileMeshComponentConnection::addConnection(const BoundaryName & boundary_name)
{
  const size_t oparenthesis_pos = boundary_name.find('(');
  if (oparenthesis_pos != std::string::npos)
  {
    logError("You are using the old connection format 'comp_name(end)'. Please update your input "
             "file to the new one 'comp_name:end'.");
  }
  else
  {
    const size_t begin_colon_pos = boundary_name.find(':');
    const size_t end_colon_pos = boundary_name.rfind(':');
    if (begin_colon_pos == end_colon_pos)
      mooseError("Boundary name ",
                 boundary_name,
                 " should include two semicolons to match this format: "
                 "component_name:boundary_name:in/out");
    // if it has a colon, assume 'component_name:end_type' format
    if (end_colon_pos != std::string::npos)
    {
      const std::string connected_component_name = boundary_name.substr(0, begin_colon_pos);
      const std::string boundary_name_cut = boundary_name.substr(0, end_colon_pos);
      const std::string str_end =
          boundary_name.substr(end_colon_pos + 1, boundary_name.length() - end_colon_pos - 1);
      const EEndType end_type = THM::stringToEnum<EEndType>(str_end);

      _connections.push_back(Connection(boundary_name_cut, connected_component_name, end_type));
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
FileMeshComponentConnection::checkNumberOfConnections(const unsigned int & n_connections) const
{
  if (_connections.size() != n_connections)
    logError("The number of connections (", _connections.size(), ") must equal ", n_connections);
}

const std::vector<dof_id_type> &
FileMeshComponentConnection::getNodeIDs() const
{
  checkSetupStatus(MESH_PREPARED);

  return _nodes;
}

const std::vector<BoundaryName> &
FileMeshComponentConnection::getBoundaryNames() const
{
  checkSetupStatus(MESH_PREPARED);

  return _boundary_names;
}
