//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowConnectedNodes.h"
#include "Conversion.h" // for stringify
#include "MooseError.h"

PorousFlowConnectedNodes::PorousFlowConnectedNodes() { clear(); }

void
PorousFlowConnectedNodes::clear()
{
  _still_adding_global_nodes = true;
  _min_global_id = std::numeric_limits<dof_id_type>::max();
  _max_global_id = std::numeric_limits<dof_id_type>::lowest();
  _set_of_global_ids.clear();
  _global_id.clear();
  _sequential_id.clear();
  _still_adding_connections = true;
  _neighbor_sets.clear();
  _sequential_neighbors.clear();
}

std::size_t
PorousFlowConnectedNodes::numNodes() const
{
  if (_still_adding_global_nodes)
    return _set_of_global_ids.size();
  return _global_id.size();
}

void
PorousFlowConnectedNodes::addGlobalNode(dof_id_type global_node_ID)
{
  if (!_still_adding_global_nodes)
    mooseError("PorousFlowConnectedNodes: addGlobalNode called, but _still_adding_global_nodes is "
               "false.  You possibly called finalizeAddingGlobalNodes too soon.");
  _set_of_global_ids.insert(global_node_ID);
  _min_global_id = std::min(_min_global_id, global_node_ID);
  _max_global_id = std::max(_max_global_id, global_node_ID);
}

void
PorousFlowConnectedNodes::finalizeAddingGlobalNodes()
{
  // populate the _global_id vector with the values in the set of global IDs
  _global_id.clear();
  for (const auto & n : _set_of_global_ids)
    _global_id.push_back(n);
  _still_adding_global_nodes = false;
  _set_of_global_ids.clear();

  // populate the _sequential_id
  _sequential_id.assign(_max_global_id - _min_global_id + 1, 0);
  for (std::size_t i = 0; i < _global_id.size(); ++i)
    _sequential_id[_global_id[i] - _min_global_id] = i;

  // prepare the _neighbor_sets
  _neighbor_sets.assign(_global_id.size(), std::set<dof_id_type>());
}

std::size_t
PorousFlowConnectedNodes::sizeSequential() const
{
  if (_still_adding_global_nodes)
    mooseError("PorousFlowConnectedNodes: sizeSequential called, but _still_adding_global_nodes is "
               "true.  Probably you should have called finalizeAddingGlobalNodes.");
  return _sequential_id.size();
}

dof_id_type
PorousFlowConnectedNodes::globalID(dof_id_type sequential_node_ID) const
{
  if (_still_adding_global_nodes)
    mooseError("PorousFlowConnectedNodes: globalID called, but _still_adding_global_nodes is true. "
               " Probably you should have called finalizeAddingGlobalNodes.");
  return _global_id[sequential_node_ID];
}

dof_id_type
PorousFlowConnectedNodes::sequentialID(dof_id_type global_node_ID) const
{
  if (_still_adding_global_nodes)
    mooseError("PorousFlowConnectedNodes: sequentialID called, but _still_adding_global_nodes is "
               "true.  Probably you should have called finalizeAddingGlobalNodes.");
  return _sequential_id[global_node_ID - _min_global_id];
}

void
PorousFlowConnectedNodes::addConnection(dof_id_type global_node_from, dof_id_type global_node_to)
{
  if (_still_adding_global_nodes)
    mooseError("PorousFlowConnectedNodes: addConnection called, but _still_adding_global_nodes is "
               "true.  Probably you should have called finalizeAddingGlobalNodes.");
  if (!_still_adding_connections)
    mooseError("PorousFlowConnectedNodes: addConnection called, but _still_adding_connections is "
               "false.  Probably you should have called finalizeAddingConnections.");
  _neighbor_sets[sequentialID(global_node_from)].insert(global_node_to);
}

void
PorousFlowConnectedNodes::finalizeAddingConnections()
{
  _sequential_neighbors.assign(_global_id.size(), std::vector<dof_id_type>());
  _global_neighbors.assign(_global_id.size(), std::vector<dof_id_type>());
  for (std::size_t i = 0; i < _global_id.size(); ++i)
    for (const auto & n : _neighbor_sets[i])
    {
      _sequential_neighbors[i].push_back(sequentialID(n));
      _global_neighbors[i].push_back(n);
    }
  _still_adding_connections = false;
  _neighbor_sets.clear();
}

const std::vector<dof_id_type> &
PorousFlowConnectedNodes::sequentialConnectionsToGlobalID(dof_id_type global_node_ID) const
{
  if (_still_adding_connections)
    mooseError("PorousFlowConnectedNodes: sequentialConnectionsToGlobalID called, but "
               "_still_adding_connections is true.  Probably you should have called "
               "finalizeAddingConnections.");
  return _sequential_neighbors[sequentialID(global_node_ID)];
}

const std::vector<dof_id_type> &
PorousFlowConnectedNodes::sequentialConnectionsToSequentialID(dof_id_type sequential_node_ID) const
{
  if (_still_adding_connections)
    mooseError("PorousFlowConnectedNodes: sequentialConnectionsToSequentialID called, but "
               "_still_adding_connections is true.  Probably you should have called "
               "finalizeAddingConnections.");
  return _sequential_neighbors[sequential_node_ID];
}

const std::vector<dof_id_type> &
PorousFlowConnectedNodes::globalConnectionsToGlobalID(dof_id_type global_node_ID) const
{
  if (_still_adding_connections)
    mooseError("PorousFlowConnectedNodes: globalConnectionsToGlobalID called, but "
               "_still_adding_connections is true.  Probably you should have called "
               "finalizeAddingConnections.");
  return _global_neighbors[sequentialID(global_node_ID)];
}

const std::vector<dof_id_type> &
PorousFlowConnectedNodes::globalConnectionsToSequentialID(dof_id_type sequential_node_ID) const
{
  if (_still_adding_connections)
    mooseError("PorousFlowConnectedNodes: globalConnectionsToSequentialID called, but "
               "_still_adding_connections is true.  Probably you should have called "
               "finalizeAddingConnections.");
  return _global_neighbors[sequential_node_ID];
}

const std::vector<dof_id_type> &
PorousFlowConnectedNodes::globalIDs() const
{
  if (_still_adding_global_nodes)
    mooseError("PorousFlowConnectedNodes: globalIDs called, but _still_adding_global_nodes is "
               "true.  Probably you should have called finalizeAddingGlobalNodes.");
  return _global_id;
}

unsigned
PorousFlowConnectedNodes::indexOfGlobalConnection(dof_id_type global_node_ID_from,
                                                  dof_id_type global_node_ID_to) const
{
  if (_still_adding_connections)
    mooseError(
        "PorousFlowConnectedNodes: indexOfGlobalConnection called, but _still_adding_connections "
        "is true.  Probably you should have called finalizeAddingConnections.");
  const std::vector<dof_id_type> con = _global_neighbors[sequentialID(global_node_ID_from)];
  const auto it = std::find(con.begin(), con.end(), global_node_ID_to);
  if (it == con.end())
    mooseError("PorousFlowConnectedNode: global_node_ID_from " +
               Moose::stringify(global_node_ID_from) + " has no connection to global_node_ID_to " +
               Moose::stringify(global_node_ID_to));
  return std::distance(con.begin(), it);
}

unsigned
PorousFlowConnectedNodes::indexOfSequentialConnection(dof_id_type sequential_node_ID_from,
                                                      dof_id_type sequential_node_ID_to) const
{
  if (_still_adding_connections)
    mooseError("PorousFlowConnectedNodes: indexOfSequentialConnection called, but "
               "_still_adding_connections is true.  Probably you should have called "
               "finalizeAddingConnections.");
  const std::vector<dof_id_type> con = _sequential_neighbors[sequential_node_ID_from];
  const auto it = std::find(con.begin(), con.end(), sequential_node_ID_to);
  if (it == con.end())
    mooseError("PorousFlowConnectedNode: sequential_node_ID_from " +
               Moose::stringify(sequential_node_ID_from) +
               " has no connection to sequential_node_ID_to " +
               Moose::stringify(sequential_node_ID_to));
  return std::distance(con.begin(), it);
}
