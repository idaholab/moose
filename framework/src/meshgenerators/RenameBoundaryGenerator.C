//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RenameBoundaryGenerator.h"
#include "CastUniquePointer.h"
#include "MooseMeshUtils.h"

#include "libmesh/mesh_modification.h"

#include <set>
#include <sstream>

registerMooseObject("MooseApp", RenameBoundaryGenerator);

InputParameters
RenameBoundaryGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");

  params.addRequiredParam<std::vector<BoundaryName>>(
      "old_boundary",
      "Elements with these boundary ID(s)/name(s) will be given the new boundary information "
      "specified in 'new_boundary'");
  params.addRequiredParam<std::vector<BoundaryName>>(
      "new_boundary",
      "The new boundary ID(s)/name(s) to be given by the boundary elements defined in "
      "'old_boundary'.");

  params.addClassDescription(
      "Changes the boundary IDs and/or boundary names for a given set of "
      "boundaries defined by either boundary ID or boundary name. The "
      "changes are independent of ordering. The merging of boundaries is supported.");

  return params;
}

RenameBoundaryGenerator::RenameBoundaryGenerator(const InputParameters & params)
  : MeshGenerator(params), _input(getMesh("input"))
{
  _old_boundary = getParam<std::vector<BoundaryName>>("old_boundary");

  _new_boundary = getParam<std::vector<BoundaryName>>("new_boundary");

  if (_old_boundary.size() != _new_boundary.size())
    paramError("new_boundary", "Must be the same length as 'old_boundary'");
}

std::unique_ptr<MeshBase>
RenameBoundaryGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  auto & boundary_info = mesh->get_boundary_info();

  // Get the current boundary IDs - take a copy so that we can also use it
  // to keep track of boundaries that we add before adding them to nodes/sides
  std::set<BoundaryID> boundary_ids = boundary_info.get_boundary_ids();
  // Take the union just in case someone else has added new boundaries in a
  // non-replicated manner
  mesh->comm().set_union(boundary_ids);

  // Helper for getting an unused boundary ID, and keeping track of it
  // so that we can generate more later
  auto get_unused_boundary_id = [this, &boundary_ids, &boundary_info]()
  {
    for (BoundaryID id = 0; id != Moose::INVALID_BOUNDARY_ID; ++id)
    {
      if (!boundary_ids.count(id) && !boundary_info.get_sideset_name_map().count(id) &&
          !boundary_info.get_nodeset_name_map().count(id))
      {
        boundary_ids.insert(id);
        return id;
      }
    }

    mooseError("Failed to find an unused ID!");
  };

  // Helper for checking whether or not a BoundaryName (which could be an ID or a name)
  // is really input as an ID
  const auto is_boundary_id = [](const BoundaryName & boundary_name)
  { return MooseUtils::isDigits(boundary_name); };

  const auto num_boundaries = _old_boundary.size();

  // Get the old boundary IDs and make sure they exist
  std::vector<BoundaryID> old_boundary_ids(num_boundaries, Moose::INVALID_BOUNDARY_ID);
  std::vector<std::string> old_boundary_names(num_boundaries);
  std::stringstream missing_boundary;
  for (const auto i : make_range(num_boundaries))
  {
    const BoundaryName & name = _old_boundary[i];

    // Convert the BoundaryName to an id and store
    const auto id = MooseMeshUtils::getBoundaryID(name, *mesh);
    old_boundary_ids[i] = id;

    // Boundary does not exist - store for a future error
    if (!boundary_ids.count(id))
      missing_boundary << name << " ";

    // Keep track of the boundary names
    // If this BoundaryName is an ID, try to see if it has a name set
    if (is_boundary_id(name))
    {
      old_boundary_names[i] = boundary_info.get_sideset_name(id);
      if (old_boundary_names[i].empty())
        old_boundary_names[i] = boundary_info.get_nodeset_name(id);
    }
    // If this BoundaryName is a name, use said name
    else
      old_boundary_names[i] = name;
  }
  if (missing_boundary.str().size())
    paramError("old_boundary",
               "The following boundaries were requested to be renamed, but do not exist: ",
               missing_boundary.str());

  // Get the boundary IDs that we're moving to
  std::vector<BoundaryID> new_boundary_ids(num_boundaries, Moose::INVALID_BOUNDARY_ID);
  std::map<BoundaryID, std::string> new_names;
  for (const auto i : make_range(num_boundaries))
  {
    const BoundaryName & name = _new_boundary[i];

    // If the user input an ID, we have the ID
    if (is_boundary_id(name))
    {
      const auto id = MooseMeshUtils::getBoundaryID(name, *mesh);
      new_boundary_ids[i] = id;

      // In the case that this is a new boundary ID, keep track of it so that we
      // don't reuse it if we have to create temproraries
      boundary_ids.insert(id);

      // Preserve the old boundary name if there was one
      if (old_boundary_names[i].size())
      {
        // if the name was the same as the ID, change the name as well
        if (old_boundary_names[i] == std::to_string(old_boundary_ids[i]))
          new_names[id] = std::to_string(new_boundary_ids[i]);
        else
          new_names[id] = old_boundary_names[i];
      }
    }
    // If the user input a name, we will use the ID that it is coming from for the
    // "new" name if the new name does not name a current boundary. If the name does
    // exist, we will merge with said boundary.
    else
    {
      bool name_already_exists = false;

      // If the target boundary already exists, merge into that one
      // Check both the old maps and the new map
      for (const auto map : {&boundary_info.set_sideset_name_map(),
                             &boundary_info.set_nodeset_name_map(),
                             &new_names})
        for (const auto & id_name_pair : *map)
          if (!name_already_exists && id_name_pair.second == name)
          {
            new_boundary_ids[i] = id_name_pair.first;
            new_names[id_name_pair.first] = name;
            name_already_exists = true;
          }

      // Target name doesn't exist, so use the source id/name
      if (!name_already_exists)
      {
        new_boundary_ids[i] = old_boundary_ids[i];
        new_names[new_boundary_ids[i]] = name;
      }
    }
  }

  // Create temproraries if needed; recall that this generator is independent
  // of input ordering and does _not_ merge sidesets.
  //
  // Take the example where we want to move 0 -> 1 and 1 -> 2. If we just
  // move them in order, we will actually end up with (0, 1) -> 2. This is
  // bad. In this case, we want to first make a temprorary for 1 (call it 3).
  // We then do: 0 -> 3, 1 -> 2, 3 -> 1 in order to get the desired behavior.
  // We will accomplish this by creating temproraries as needed, modifying
  // the initial move to the temproraries as needed, and then moving the
  // temproraries back. temp_change_ids here are the (from -> to) pairs
  // that we will move at the end.
  auto temp_new_boundary_ids = new_boundary_ids;
  std::vector<std::pair<BoundaryID, BoundaryID>> temp_change_ids;
  // Loop through all new IDs
  for (const auto new_i : make_range(num_boundaries))
  {
    // Look at all of the old IDs that will be moved after the move to the new ID.
    // If any of the old IDs after are IDs that we are moving to, create a temprorary
    // and keep track of it so we can move it back at the end.
    for (const auto old_i : make_range(new_i + 1, num_boundaries))
      if (new_boundary_ids[new_i] == old_boundary_ids[old_i])
      {
        const auto temp_id = get_unused_boundary_id();
        temp_change_ids.emplace_back(temp_id, new_boundary_ids[new_i]);
        temp_new_boundary_ids[new_i] = temp_id;
        break;
      }
  }

  // First pass through changing the boundary ids
  for (const auto i : make_range(num_boundaries))
    MeshTools::Modification::change_boundary_id(
        *mesh, old_boundary_ids[i], temp_new_boundary_ids[i]);

  // Pass through moving the temproraries to the actual boundaries, if necessary
  for (const auto & pair : temp_change_ids)
    MeshTools::Modification::change_boundary_id(*mesh, pair.first, pair.second);

  // First go through and remove all of the old names
  for (const auto i : make_range(num_boundaries))
  {
    if (boundary_info.get_sideset_name_map().count(old_boundary_ids[i]))
      boundary_info.set_sideset_name_map().erase(old_boundary_ids[i]);
    if (boundary_info.get_nodeset_name_map().count(old_boundary_ids[i]))
      boundary_info.set_nodeset_name_map().erase(old_boundary_ids[i]);
  }

  // With the old names removed, add the new names if there are any to add
  for (const auto & pair : new_names)
  {
    boundary_info.sideset_name(pair.first) = pair.second;
    boundary_info.nodeset_name(pair.first) = pair.second;
  }

  mesh->set_isnt_prepared();
  return dynamic_pointer_cast<MeshBase>(mesh);
}
