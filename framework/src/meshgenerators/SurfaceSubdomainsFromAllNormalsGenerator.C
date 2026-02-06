//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SurfaceSubdomainsFromAllNormalsGenerator.h"
#include "Parser.h"
#include "InputParameters.h"
#include "CastUniquePointer.h"

#include "libmesh/fe_base.h"
#include "libmesh/mesh_generation.h"
#include "libmesh/mesh.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/point_locator_base.h"
#include "libmesh/distributed_mesh.h"
#include "libmesh/elem.h"

#include <typeinfo>

registerMooseObject("MooseApp", SurfaceSubdomainsFromAllNormalsGenerator);

InputParameters
SurfaceSubdomainsFromAllNormalsGenerator::validParams()
{
  InputParameters params = SubdomainsGeneratorBase::validParams();

  params.addParam<bool>(
      "contiguous_assignments_only",
      false,
      "Whether to only group elements in a subdomain using the 'flooding' algorithm");
  // There can be many of them
  params.suppressParameter<std::vector<SubdomainName>>("new_subdomain");
  // Using normals is the base principle of this mesh generator
  params.addPrivateParam<bool>("_using_normal", true);

  params.addClassDescription(
      "Adds subdomains to surface (2D) elements in the (3D) mesh based on unique normals.");
  return params;
}

SurfaceSubdomainsFromAllNormalsGenerator::SurfaceSubdomainsFromAllNormalsGenerator(
    const InputParameters & parameters)
  : SubdomainsGeneratorBase(parameters), _flood_only(getParam<bool>("contiguous_assignments_only"))
{
}

std::unique_ptr<MeshBase>
SurfaceSubdomainsFromAllNormalsGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);
  if (!mesh->is_replicated())
    mooseError(
        "SurfaceSubdomainsFromAllNormalsGenerator is not implemented for distributed meshes");
  setup(*mesh);

  _visited.clear();
  unsigned int num_neighborless = 0;

  // We'll need to loop over all of the elements to find ones that match this normal.
  // We can't rely on flood catching them all here...
  for (auto & elem : mesh->element_ptr_range())
  {
    // Nothing to do with edges
    if (elem->n_nodes() < 3)
      continue;
    // Nothing to do with 3D elements
    if (elem->dim() > 2)
      continue;
    // Likely an issue with the mesh
    if (_flood_only && elem->n_neighbors() == 0)
    {
      num_neighborless++;
      mooseWarning("Element '" + std::to_string(elem->id()) + "' has no neighbors");
    }

    // Compute the normal
    const auto & p1 = elem->point(0);
    const auto & p2 = elem->point(1);
    const auto & p3 = elem->point(2);
    auto normal = (p1 - p2).cross(p1 - p3);
    if (normal.norm_sq() == 0)
    {
      mooseWarning("Colinear nodes on elements, skipping subdomain assignment for this element");
      continue;
    }
    normal = normal.unit();

    // See if we've seen this normal before (linear search)
    const std::map<SubdomainID, RealVectorValue>::value_type * item = nullptr;
    if (!_flood_only)
      for (const auto & id_pair : _subdomain_to_normal_map)
        if (normalsWithinTol(id_pair.second, normal, _normal_tol))
        {
          item = &id_pair;
          break;
        }

    // Flood with the previously created subdomains and normals
    if (item)
      flood(elem, item->second, item->first);
    // Flood with a new subdomain and the element normal
    else
    {
      subdomain_id_type id = MooseMeshUtils::getNextFreeSubdomainID(*mesh);
      _subdomain_to_normal_map[id] = normal;
      flood(elem, normal, id);
    }
  }

  if (_flood_only && num_neighborless)
    mooseWarning("Several subdomains were created for neighborless elements: " +
                 std::to_string(num_neighborless));

  mesh->set_isnt_prepared();
  return dynamic_pointer_cast<MeshBase>(mesh);
}
