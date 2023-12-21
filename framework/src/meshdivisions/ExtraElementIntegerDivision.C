//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExtraElementIntegerDivision.h"
#include "MooseMesh.h"

#include "libmesh/mesh_base.h"
#include "libmesh/elem.h"

registerMooseObject("MooseApp", ExtraElementIntegerDivision);

InputParameters
ExtraElementIntegerDivision::validParams()
{
  InputParameters params = MeshDivision::validParams();
  params.addClassDescription(
      "Divide the mesh by increasing extra element IDs. The division will be contiguously "
      "numbered even if the extra element ids are not");
  params.addRequiredParam<ExtraElementIDName>("extra_id_name",
                                              "The name of the extra element ID in the mesh");
  return params;
}

ExtraElementIntegerDivision::ExtraElementIntegerDivision(const InputParameters & parameters)
  : MeshDivision(parameters), _extra_id_name(getParam<ExtraElementIDName>("extra_id_name"))
{
  if (!_mesh.getMesh().has_elem_integer(_extra_id_name))
    paramError("extra_id_name", "The source element ID does not exist on the input mesh");
  _extra_id = _mesh.getMesh().get_elem_integer_index(_extra_id_name);

  ExtraElementIntegerDivision::initialize();
  _mesh_fully_indexed = true;
}

void
ExtraElementIntegerDivision::initialize()
{
  // The ExtraElementID may not be contiguously numbered so we gather them all first
  std::set<libMesh::dof_id_type> extra_ids;
  for (const auto & elem : _mesh.getMesh().active_local_element_ptr_range())
    extra_ids.insert(elem->get_extra_integer(_extra_id));
  _mesh.comm().set_union(extra_ids);
  setNumDivisions(extra_ids.size());

  unsigned int i = 0;
  for (const auto extra_id : extra_ids)
    _extra_ids_to_division_index[extra_id] = i++;
}

unsigned int
ExtraElementIntegerDivision::divisionIndex(const Elem & elem) const
{
  return libmesh_map_find(_extra_ids_to_division_index, elem.get_extra_integer(_extra_id));
}

unsigned int
ExtraElementIntegerDivision::divisionIndex(const Point & pt) const
{
  const auto & pl = _mesh.getMesh().sub_point_locator();
  // There could be more than one elem if we are on the edge between two elements
  std::set<const Elem *> candidates;
  (*pl)(pt, candidates);

  // By convention we will use the element with the lowest extra element id
  // There could be other conventions: use lowest element if for example
  const Elem * elem = nullptr;
  libMesh::dof_id_type min_extra_id = std::numeric_limits<int>::max();
  for (const auto elem_ptr : candidates)
    if (elem_ptr->get_extra_integer(_extra_id) < min_extra_id)
    {
      elem = elem_ptr;
      min_extra_id = elem_ptr->get_extra_integer(_extra_id);
    }

  return libmesh_map_find(_extra_ids_to_division_index, elem->get_extra_integer(_extra_id));
}
