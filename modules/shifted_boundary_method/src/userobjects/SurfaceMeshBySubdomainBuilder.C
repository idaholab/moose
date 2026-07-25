//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SurfaceMeshBySubdomainBuilder.h"
#include "InputParameters.h"

registerMooseObject("ShiftedBoundaryMethodApp", SurfaceMeshBySubdomainBuilder);

InputParameters
SurfaceMeshBySubdomainBuilder::validParams()
{
  InputParameters params = BoundaryMeshBuilder::validParams();
  params.addClassDescription(
      "Builds one SurfaceElementSet per subdomain from a given surface mesh.");
  return params;
}

SurfaceMeshBySubdomainBuilder::SurfaceMeshBySubdomainBuilder(const InputParameters & parameters)
  : BoundaryMeshBuilder(parameters)
{
}

void
SurfaceMeshBySubdomainBuilder::buildDefaultSet()
{
  // Group active elements by subdomain, then build one SurfaceElementSet per group.
  std::unordered_map<subdomain_id_type, std::vector<const Elem *>> elems_by_subdomain;
  for (const auto * elem : _mesh->active_element_ptr_range())
    elems_by_subdomain[elem->subdomain_id()].push_back(elem);

  for (const auto & [sid, elems] : elems_by_subdomain)
    _sets_by_subdomain.emplace(sid, SurfaceElementSet::fromElements(elems));
}
