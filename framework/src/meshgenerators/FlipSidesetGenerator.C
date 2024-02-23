//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlipSidesetGenerator.h"

#include "CastUniquePointer.h"

registerMooseObject("MooseApp", FlipSidesetGenerator);

InputParameters
FlipSidesetGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addClassDescription("A Mesh Generator which flips a given sideset");
  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addRequiredParam<BoundaryName>("boundary", "The sideset (boundary) that will be flipped");
  return params;
}

FlipSidesetGenerator::FlipSidesetGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _sideset_name(getParam<BoundaryName>("boundary"))
{
}

std::unique_ptr<MeshBase>
FlipSidesetGenerator::generate()
{
  // get boundary info
  BoundaryInfo & boundary_info = _input->get_boundary_info();
  // get id of the input sideset
  const auto sideset_id = boundary_info.get_id_by_name(_sideset_name);

  // Throw an error if the sideset doesn't exist
  if (sideset_id == libMesh::BoundaryInfo::invalid_id)
    paramError("boundary", "The boundary '", _sideset_name, "' was not found");

  // get a copy of sideset map to avoid changing the sideset map while looping on it
  std::multimap<const Elem *, std::pair<unsigned short int, boundary_id_type>> sideset_map =
      boundary_info.get_sideset_map();

  // old_elem is the original element attached to the sideset before flipping
  // new_elem is the element attached to the sideset after flipping
  for (const auto & [old_elem, id_pair] : sideset_map)
  {
    boundary_id_type boundary_id = std::get<1>(id_pair);
    if (boundary_id == sideset_id)
    {
      const auto old_side_id = std::get<0>(id_pair);
      const auto old_elem_id = old_elem->id();
      const auto new_elem = old_elem->neighbor_ptr(old_side_id);

      // Throw an error if the old element doesn't have a neighbor on the old side
      if (!new_elem)
        mooseError("elem " + std::to_string(old_elem_id) +
                   " does not have a neighbor through side " + std::to_string(old_side_id) +
                   " therefore it cannot be flipped");

      const auto new_side_id = new_elem->which_neighbor_am_i(old_elem);
      boundary_info.remove_side(old_elem, old_side_id, sideset_id);
      boundary_info.add_side(new_elem, new_side_id, sideset_id);
    }
  }
  return dynamic_pointer_cast<MeshBase>(_input);
}
