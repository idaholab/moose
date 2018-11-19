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

registerMooseObject("MooseApp", RenameBoundaryGenerator);

template <>
InputParameters
validParams<RenameBoundaryGenerator>()
{
  InputParameters params = validParams<MeshGenerator>();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addParam<std::vector<boundary_id_type>>(
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
  params.addParam<std::vector<boundary_id_type>>(
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
    _old_boundary_id = getParam<std::vector<boundary_id_type>>("old_boundary_id");
    _old_boundary_name.reserve(_old_boundary_id.size());
    for (const auto & boundary_id : _old_boundary_id)
      _old_boundary_name.emplace_back(boundary_info.get_sideset_name(boundary_id));
  }
  else
  {
    // user must have supplied old_boundary_name
    _old_boundary_name = getParam<std::vector<BoundaryName>>("old_boundary_name");
    _old_boundary_id.reserve(_old_boundary_name.size());
    for (const auto & boundary_name : _old_boundary_name)
      _old_boundary_id.emplace_back(mesh->get_id_by_name(boundary_name));
  }

  // construct new_boundary_id and new_boundary_name
  if (isParamValid("new_boundary_id"))
  {
    _new_boundary_id = getParam<std::vector<boundary_id_type>>("new_boundary_id");
    if (_new_boundary_id.size() != _old_boundary_id.size())
      mooseError(
          "RenameBoundaryGenerator: The vector of old_boundary information must have the same"
          " length as the vector of new_boundary information\n");

    // construct the _new_boundary_name
    _new_boundary_name.reserve(_new_boundary_id.size());
    for (const auto & boundary_id : _new_boundary_id)
      _new_boundary_name.emplace_back(newBoundaryName(boundary_id));
  }
  else // must have supplied new_boundary_name
  {
    _new_boundary_name = getParam<std::vector<BoundaryName>>("new_boundary_name");
    if (_new_boundary_name.size() != _old_boundary_id.size())
      mooseError(
          "RenameBoundaryGenerator: The vector of old_boundary information must have the same"
          " length as the vector of new_boundary information\n");

    _new_boundary_id.reserve(_new_boundary_name.size());
    for (const auto & boundary_name : _new_boundary_name)
      _new_boundary_id.emplace_back(newBoundaryID(boundary_name));
  }

  for (const auto & elem : mesh->active_element_ptr_range())
  {
    for (unsigned int side = 0; side < elem->n_sides(); side++)
    {
      for (unsigned i = 0; i < _old_boundary_id.size(); ++i)
        if (boundary_info.has_boundary_id(elem, side, _old_boundary_id[i]))
        {
          boundary_info.remove_side(elem, side);
          boundary_info.add_side(elem, side, _new_boundary_id[i]);
        }
    }
  }
  for (unsigned i = 0; i < _old_boundary_id.size(); ++i)
    if (_new_boundary_name[i].size() > 0)
      boundary_info.sideset_name(_old_boundary_id[i]) = _new_boundary_name[i];

  return dynamic_pointer_cast<MeshBase>(mesh);
}

BoundaryName
RenameBoundaryGenerator::newBoundaryName(const boundary_id_type & new_boundary_id)
{
  for (unsigned i = 0; i < _new_boundary_id.size(); ++i)
    if (_new_boundary_id[i] == new_boundary_id)
      return _old_boundary_name[i];
  mooseError("RenameBoundaryGenerator: error in code");
}

boundary_id_type
RenameBoundaryGenerator::newBoundaryID(const BoundaryName & new_boundary_name)
{
  for (unsigned i = 0; i < _new_boundary_name.size(); ++i)
    if (_new_boundary_name[i] == new_boundary_name)
      return _old_boundary_id[i];
  mooseError("RenameBoundaryGenerator: error in code");
}
