//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RenameBoundaryGenerator.h"
#include "CastUniquePointer.h"

#include "libmesh/mesh_modification.h"

#include <set>
#include <algorithm>
#include <sstream>

registerMooseObject("MooseApp", RenameBoundaryGenerator);

defineLegacyParams(RenameBoundaryGenerator);

InputParameters
RenameBoundaryGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addParam<std::vector<BoundaryID>>(
      "old_boundary_id",
      "Elements with this boundary number will be given the new_boundary_number or "
      "new_boundary_name.  You must supply either old_boundary_id or old_boundary_name.  "
      "You may supply a vector of old_boundary_id, in which case the new_boundary "
      "information must also be a vector.");
  params.addParam<std::vector<BoundaryName>>(
      "old_boundary_name",
      "Elements with this boundary name will be given the new_boundary_number or "
      "new_boundary_name.  You must supply either old_boundary_id or old_boundary_name.  "
      "You may supply a vector of old_boundary_name, in which case the new_boundary "
      "information must also be a vector.");
  params.addParam<std::vector<BoundaryID>>(
      "new_boundary_id",
      "Elements with the old boundary number (or name) will be given this boundary "
      "number.  If the old boundaries are named, their names will be passed onto the "
      "newly numbered boundarys.");
  params.addParam<std::vector<BoundaryName>>(
      "new_boundary_name",
      "Elements with the old boundary number (or name) will be given this boundary "
      "name.  No change of boundary ID is performed, unless multiple old boundaries are "
      "given the same name, in which case they are all given the first old boundary "
      "number.");
  params.addClassDescription(
      "RenameBoundaryGenerator re-numbers or re-names an old_boundary_id or old_boundary_name "
      "with a new_boundary_id or new_boundary_name.  If using RenameBoundaryGenerator to "
      "merge boundaries (by giving them the same name, for instance) it is "
      "advisable to specify all your boundaries in old_boundaries to avoid inconsistencies");

  return params;
}

RenameBoundaryGenerator::RenameBoundaryGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters), _input(getMesh("input"))
{
  // error checking.  Must have exactly one of old_boundary_id or old_boundary_name
  if (isParamValid("old_boundary_id") && isParamValid("old_boundary_name"))
    mooseError("RenameBoundaryGenerator: You must supply exactly one of old_boundary_id or "
               "old_boundary_name\n");
  else if (!isParamValid("old_boundary_id") && !isParamValid("old_boundary_name"))
    mooseError("RenameBoundaryGenerator: You must supply exactly one of old_boundary_id or "
               "old_boundary_name\n");

  // error checking.  Must have exactly one of new_boundary_id or new_boundary_name
  // In principal we could have both (the old boundary would then be given a new ID and a new name)
  // but i feel that could lead to confusion for the user.  If the user wants to do that they
  // should use two of these RenameBoundary MeshModifiers.
  if (isParamValid("new_boundary_id") && isParamValid("new_boundary_name"))
    mooseError("RenameBoundaryGenerator: You must supply exactly one of new_boundary_id or "
               "new_boundary_name\n");
  else if (!isParamValid("new_boundary_id") && !isParamValid("new_boundary_name"))
    mooseError("RenameBoundaryGenerator: You must supply exactly one of new_boundary_id or "
               "new_boundary_name\n");
}

std::unique_ptr<MeshBase>
RenameBoundaryGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  auto & boundary_info = mesh->get_boundary_info();

  // grab the user input.  Can't do all this in the constructor as some things may not
  // have been put into the mesh yet, eg old_boundary_name might have been inserted by
  // another MeshGenerator
  if (isParamValid("old_boundary_id"))
  {
    // user must have supplied old_boundary_id
    _old_boundary_id = getParam<std::vector<BoundaryID>>("old_boundary_id");
  }

  if (isParamValid("old_boundary_name"))
  {
    _old_boundary_name = getParam<std::vector<BoundaryName>>("old_boundary_name");

    _old_boundary_id.reserve(_old_boundary_name.size());

    for (auto & old_name : _old_boundary_name)
      _old_boundary_id.emplace_back(boundary_info.get_id_by_name(old_name));
  }

  // Sort the ids for use in the error check (make a copy so we don't mess up original order)
  const std::set<BoundaryID> old_ids(_old_boundary_id.begin(), _old_boundary_id.end());

  const std::set<BoundaryID> & mesh_boundary_ids = boundary_info.get_boundary_ids();

  std::set<BoundaryID> difference;
  std::set_difference(old_ids.begin(),
                      old_ids.end(),
                      mesh_boundary_ids.begin(),
                      mesh_boundary_ids.end(),
                      std::inserter(difference, difference.end()));
  if (!difference.empty())
  {
    std::stringstream missing_boundary_ids;
    std::copy(difference.begin(),
              difference.end(),
              std::ostream_iterator<unsigned int>(missing_boundary_ids, " "));
    paramError("old_boundary_id",
               "The following boundary IDs were requested to be renamed, but do not exist: " +
                   missing_boundary_ids.str());
  }

  if (isParamValid("new_boundary_id"))
  {
    _new_boundary_id = getParam<std::vector<BoundaryID>>("new_boundary_id");

    if (_new_boundary_id.size() != _old_boundary_id.size())
      mooseError(
          "RenameBoundaryGenerator: The vector of old_boundary information must have the same"
          " length as the vector of new_boundary information\n");

    for (unsigned int i = 0; i < _new_boundary_id.size(); i++)
      MeshTools::Modification::change_boundary_id(*mesh, _old_boundary_id[i], _new_boundary_id[i]);

    return dynamic_pointer_cast<MeshBase>(mesh);
  }

  if (isParamValid("new_boundary_name"))
  {
    _new_boundary_name = getParam<std::vector<BoundaryName>>("new_boundary_name");

    if (_new_boundary_name.size() != _old_boundary_id.size())
      mooseError(
          "RenameBoundaryGenerator: The vector of old_boundary information must have the same"
          " length as the vector of new_boundary information\n");

    for (unsigned int i = 0; i < _new_boundary_name.size(); i++)
    {
      boundary_info.sideset_name(_old_boundary_id[i]) = _new_boundary_name[i];
      boundary_info.nodeset_name(_old_boundary_id[i]) = _new_boundary_name[i];
    }
    return dynamic_pointer_cast<MeshBase>(mesh);
  }

  mooseError("Must supply one of either new_boundary_id or new_boundary_name");
}
