//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SubdomainsDivision.h"
#include "MooseMesh.h"

#include "libmesh/mesh_base.h"
#include "libmesh/elem.h"

registerMooseObject("MooseApp", SubdomainsDivision);

InputParameters
SubdomainsDivision::validParams()
{
  InputParameters params = MeshDivision::validParams();
  params += BlockRestrictable::validParams();
  params.addClassDescription(
      "Divide the mesh by increasing subdomain ids. The division will be contiguously "
      "numbered even if the subdomain ids are not");
  return params;
}

SubdomainsDivision::SubdomainsDivision(const InputParameters & parameters)
  : MeshDivision(parameters), BlockRestrictable(this)
{
  SubdomainsDivision::initialize();
  _mesh_fully_indexed = true;
}

void
SubdomainsDivision::initialize()
{
  if (!blockRestricted())
  {
    // The subdomains may not be contiguously numbered
    const auto & subdomain_ids = blockIDs();
    setNumDivisions(subdomain_ids.size());
    unsigned int i = 0;
    for (const auto sub_id : subdomain_ids)
      _subdomain_ids_to_division_index[sub_id] = i++;
  }
  else
  {
    // Follow the user input order, use a vector instead of a set
    const auto subdomain_ids = _mesh.getSubdomainIDs(blocks());
    setNumDivisions(subdomain_ids.size());
    unsigned int i = 0;
    for (const auto sub_id : subdomain_ids)
      _subdomain_ids_to_division_index[sub_id] = i++;
  }
}

unsigned int
SubdomainsDivision::divisionIndex(const Elem & elem) const
{
  const auto id_it = _subdomain_ids_to_division_index.find(elem.subdomain_id());
  if (id_it != _subdomain_ids_to_division_index.end())
    return id_it->second;
  else
  {
    mooseAssert(blockRestricted(), "We should be block restricted");
    return MooseMeshDivision::INVALID_DIVISION_INDEX;
  }
}

unsigned int
SubdomainsDivision::divisionIndex(const Point & pt) const
{
  const auto & pl = _mesh.getMesh().sub_point_locator();
  // There could be more than one elem if we are on the edge between two elements
  std::set<const Elem *> candidates;
  (*pl)(pt, candidates);

  // By convention we will use the element with the lowest subdomain id
  const Elem * elem = nullptr;
  unsigned int min_sub_id = Moose::INVALID_BLOCK_ID;
  for (const auto elem_ptr : candidates)
    if (elem_ptr->subdomain_id() < min_sub_id)
    {
      elem = elem_ptr;
      min_sub_id = elem_ptr->subdomain_id();
    }

  return libmesh_map_find(_subdomain_ids_to_division_index, elem->subdomain_id());
}
