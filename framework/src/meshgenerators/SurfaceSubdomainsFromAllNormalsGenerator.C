//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SurfaceSubdomainsFromAllNormalsGenerator.h"
#include "InputParameters.h"
#include "CastUniquePointer.h"

#include "libmesh/mesh_generation.h"
#include "libmesh/mesh.h"
#include "libmesh/string_to_enum.h"
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
      "Whether to only group elements in a subdomain using the 'flooding' algorithm. "
      "We strongly recommend pairing this with the 'flood_elements_once' parameter");
  params.addParam<bool>(
      "select_max_neighbor_element_subdomains",
      false,
      "Whether to perform a final subdomain assignment the element is assigned to the subdomain "
      "that holds the most neighbors of the element with a similar normal");

  // There can be many of them, and we don't control the number in this generator
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

  unsigned int num_neighborless = 0;

  // We'll need to loop over all of the elements to find ones that match this normal.
  // We can't rely on flood catching them all in one go. We have to flood from multiple elements
  // in case there are disconnected groups of elements.
  // Depending on user parameters, we may be "flooding" multiple times the same elements
  for (auto & elem : mesh->element_ptr_range())
  {
    // Nothing to do with edges
    if (elem->dim() < 2)
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

    // Check if element should be used to paint from
    if (_included_subdomain_ids.size() &&
        std::find(_included_subdomain_ids.begin(),
                  _included_subdomain_ids.end(),
                  elem->subdomain_id()) == _included_subdomain_ids.end())
      continue;

    // Compute the normal
    const auto normal = get2DElemNormal(elem);

    // See if we've seen this normal before (linear search)
    const std::map<SubdomainID, RealVectorValue>::value_type * item = nullptr;
    bool sub_id_found = false;
    SubdomainID sub_id;
    if (!_flood_only)
      for (const auto & id_pair : _subdomain_to_normal_map)
        if (normalsWithinTol(id_pair.second, normal, _normal_tol) ||
            (_allow_normal_flips && normalsWithinTol(id_pair.second, normal, _normal_tol)))
        {
          sub_id_found = true;
          item = &id_pair;
          break;
        }
    if (_check_painted_neighor_normals)
    {
      std::map<SubdomainID, unsigned int> sub_id_neighbors;
      // Try to flood from each side with the same subdomain
      // Look for the neighbor subdomain id with the most neighbors
      // NOTE: we could exceed max distance here
      for (const auto neighbor : make_range(elem->n_sides()))
        if (elem->neighbor_ptr(neighbor) &&
            _acted_upon_once.find(elem->neighbor_ptr(neighbor)) != _acted_upon_once.end() &&
            elementSatisfiesRequirements(elem,
                                         get2DElemNormal(elem->neighbor_ptr(neighbor)),
                                         *elem->neighbor_ptr(neighbor),
                                         normal))
        {
          sub_id_found = true;
          sub_id_neighbors[elem->neighbor_ptr(neighbor)->subdomain_id()]++;
        }

      unsigned int max_of_subid = 0;
      for (const auto & [key, item] : sub_id_neighbors)
        if (item >= max_of_subid)
        {
          max_of_subid = item;
          sub_id = key;
        }
    }

    // Flood with the previously created subdomains and normals
    // TODO: we need to store the starting element for that patch, in case it is not "elem"
    if (item)
      flood(elem, item->second, *elem, item->first, *mesh);
    else if (sub_id_found)
      flood(elem, normal, *elem, sub_id, *mesh);
    // Flood with a new subdomain and the element normal
    else
    {
      sub_id = MooseMeshUtils::getNextFreeSubdomainID(*mesh);
      _subdomain_to_normal_map[sub_id] = normal;
      flood(elem, normal, *elem, sub_id, *mesh);
    }
  }

  // Check all elements once with only neighbor averaging
  if (getParam<bool>("select_max_neighbor_element_subdomains"))
    for (auto & elem : mesh->element_ptr_range())
    {
      // Nothing to do with edges
      if (elem->dim() < 2)
        continue;
      // Nothing to do with 3D elements
      if (elem->dim() > 2)
        continue;

      // Compute the normal
      const auto normal = get2DElemNormal(elem);

      bool sub_id_found = false;
      SubdomainID sub_id;
      std::map<SubdomainID, unsigned int> sub_id_neighbors;
      // Use point neighbors, it's easy for a surface tri3 to be sandwiched into the wrong subdomain
      std::set<const Elem *> neighbor_set;
      elem->find_point_neighbors(neighbor_set);

      // Try to flood from each side with the same subdomain
      // Look for the neighbor subdomain id with the most neighbors
      // NOTE: we might exceeed the patch distance here
      for (const auto neighbor : neighbor_set)
        if (elementSatisfiesRequirements(elem, get2DElemNormal(neighbor), *neighbor, normal))
        {
          sub_id_found = true;
          sub_id_neighbors[neighbor->subdomain_id()]++;
        }

      unsigned int max_of_subid = 0;
      for (const auto & [key, item] : sub_id_neighbors)
        if (item >= max_of_subid)
        {
          max_of_subid = item;
          sub_id = key;
        }
      if (sub_id_found)
        elem->subdomain_id() = sub_id;
    }

  if (_flood_only && num_neighborless)
    mooseWarning("Several subdomains were created for neighborless elements: " +
                 std::to_string(num_neighborless));

  mesh->set_isnt_prepared();
  return dynamic_pointer_cast<MeshBase>(mesh);
}
