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
#include "MooseMeshUtils.h"
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
  InputParameters params = SurfaceMeshGeneratorBase::validParams();

  params.renameParam("max_paint_size_centroids", "max_subdomain_size_centroids", "");
  params.addParam<bool>(
      "contiguous_assignments_only",
      false,
      "Whether to only group elements in a subdomain using the 'flooding' algorithm. "
      "We strongly recommend pairing this with the 'flood_elements_once' parameter");

  // Post flooding operations/cleanup
  params.addParam<bool>("select_max_neighbor_element_subdomains",
                        false,
                        "Whether to perform a final subdomain assignment phase where each element "
                        "is assigned to the subdomain "
                        "that holds the most neighbors of the element with a similar normal.");
  params.addParam<bool>(
      "separate_elements_connected_by_a_single_node",
      false,
      "Whether to perform a final subdomain assignment phase where any element that is not "
      "connected to other elements from the same subdomain is re-assigned to either the subdomain "
      "that all side neighbor elements belong to, or a new subdomain if there is no such subdomain "
      "including all its side neighbors. Note that normals are not checked in this phase, and the "
      "subdomain size criterion is assumed to be already met.");

  params.addParamNamesToGroup("contiguous_assignments_only", "Other flooding");
  // NOTE: these post-flooding / post-treatment operations could be moved to a base class.
  // They could also become their own mesh generator!
  params.addParamNamesToGroup(
      "separate_elements_connected_by_a_single_node select_max_neighbor_element_subdomains",
      "Post flooding subdomain re-assignments");

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
  : SurfaceMeshGeneratorBase(parameters),
    _contiguous_assignments_only(getParam<bool>("contiguous_assignments_only"))
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
  const bool user_specified_normal = isParamSetByUser("normal");

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
    if (_contiguous_assignments_only && elem->n_neighbors() == 0)
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
    const auto elem_normal = get2DElemNormal(elem);

    // Four options for which subdomain to paint with:
    // 1) See if we've seen this normal before (linear search), within some tolerance
    // NOTE: these elements might not be neighbors of the current element,
    // so the patch may not be contiguous
    const std::map<SubdomainID, RealVectorValue>::value_type * item = nullptr;
    bool sub_id_found = false;
    if (!_contiguous_assignments_only)
      for (const auto & id_pair : _subdomain_to_normal_map)
        if (elementSatisfiesRequirements(
                elem, id_pair.second, *_subdomain_to_starting_elem[id_pair.first], elem_normal))
        {
          sub_id_found = true;
          item = &id_pair;
          break;
        }
    // 2) Look at the neighbors, if a majority of them have the same subdomain and a similar normal,
    // use that subdomain to paint elements with
    // NOTE: compatible with 'contiguous_assignments_only' option
    // NOTE: this overrides the result of the previous search
    SubdomainID neighbor_majority_sub_id;
    if (_check_painted_neighor_normals)
    {
      std::map<SubdomainID, unsigned int> sub_id_neighbors;
      // Try to flood from each side with the same subdomain
      // Look for the neighbor subdomain id with the most neighbors
      for (const auto neighbor : make_range(elem->n_sides()))
        if (elem->neighbor_ptr(neighbor) &&
            _acted_upon_once.find(elem->neighbor_ptr(neighbor)) != _acted_upon_once.end() &&
            elementSatisfiesRequirements(elem,
                                         get2DElemNormal(elem->neighbor_ptr(neighbor)),
                                         *elem->neighbor_ptr(neighbor),
                                         elem_normal))
        {
          sub_id_found = true;
          sub_id_neighbors[elem->neighbor_ptr(neighbor)->subdomain_id()]++;
        }

      unsigned int max_of_subid = 0;
      for (const auto & [key, item] : sub_id_neighbors)
        if (item >= max_of_subid)
        {
          max_of_subid = item;
          neighbor_majority_sub_id = key;
        }

      // Note: the max distance from starting element of the subdomain flooding to this
      // element should be checked in the call to flood()
    }
    // 3) Flood only with the selected fixed normal
    // Note: this steers away from an "FromAllNormals" generator, it's more like a "FromNormal"
    // generator capability.
    if (user_specified_normal && !elementSatisfiesRequirements(elem, _normal, *elem, elem_normal))
      continue;
    // 4) Flood with a new subdomain every time ('contiguous_assignments_only' option)

    // Finalize flooding parameter selection
    Elem * starting_element = elem;
    subdomain_id_type flooding_sub_id;
    Point flooding_normal = elem_normal;
    if (item)
    {
      starting_element = _subdomain_to_starting_elem[item->first];
      flooding_sub_id = item->first;
      flooding_normal = item->second;
    }
    else if (sub_id_found)
    {
      starting_element = _subdomain_to_starting_elem[neighbor_majority_sub_id];
      flooding_sub_id = neighbor_majority_sub_id;
    }
    else
    {
      flooding_sub_id = MooseMeshUtils::getNextFreeSubdomainID(*mesh);
      _subdomain_to_normal_map[flooding_sub_id] = elem_normal;
      _subdomain_to_starting_elem[flooding_sub_id] = elem;
    }
    // User input of a normal takes precedence
    if (user_specified_normal)
    {
      flooding_normal = _normal;
      _subdomain_to_normal_map[flooding_sub_id] = flooding_normal;
    }

    // Flood with the previously created subdomains and normals
    flood(elem, flooding_normal, *starting_element, flooding_sub_id, *mesh);
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

      // NOTE: we use side neighbors instead of point neighbors as side neighbors
      // are always connected by 2 nodes to the rest of the subdomains, and elements
      // connected by a single node to a subdomain cause issues when triangulating
      // that subdomain.

      // Try to flood from each side with the same subdomain
      // Look for the neighbor subdomain id with the most neighbors
      for (const auto neighbor_i : make_range(elem->n_sides()))
      {
        const auto neighbor = elem->neighbor_ptr(neighbor_i);
        // NOTE: to avoid exceeding the patch size, we pass the element that was used to start
        // painting the patch to check the 'distance'/'patch size' criterion
        if (neighbor &&
            elementSatisfiesRequirements(elem,
                                         get2DElemNormal(neighbor),
                                         *_subdomain_to_starting_elem[neighbor->subdomain_id()],
                                         normal))
        {
          sub_id_found = true;
          sub_id_neighbors[neighbor->subdomain_id()]++;
        }
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

  if (getParam<bool>("separate_elements_connected_by_a_single_node"))
    for (auto & elem : mesh->element_ptr_range())
    {
      // Nothing to do with edges
      if (elem->dim() < 2)
        continue;
      // Nothing to do with 3D elements
      if (elem->dim() > 2)
        continue;

      bool connected_to_a_neighbor = false;
      for (const auto neighbor : make_range(elem->n_sides()))
        if (elem->neighbor_ptr(neighbor) &&
            elem->subdomain_id() == elem->neighbor_ptr(neighbor)->subdomain_id())
          connected_to_a_neighbor = true;

      if (!connected_to_a_neighbor)
      {
        bool same_subdomain = true;
        subdomain_id_type common_sub = std::numeric_limits<subdomain_id_type>::max();

        // If all side-neighbors have the same subdomain, use that subdomain instead
        // NOTE: we don't check the normal criteria here
        for (const auto neighbor : make_range(elem->n_sides()))
          if (elem->neighbor_ptr(neighbor))
          {
            if (common_sub == std::numeric_limits<subdomain_id_type>::max())
              common_sub = elem->neighbor_ptr(neighbor)->subdomain_id();
            else if (common_sub != elem->neighbor_ptr(neighbor)->subdomain_id())
              same_subdomain = false;
          }

        if (same_subdomain && (common_sub != std::numeric_limits<subdomain_id_type>::max()))
          elem->subdomain_id() = common_sub;
        else
          elem->subdomain_id() = MooseMeshUtils::getNextFreeSubdomainID(*mesh);
      }
    }

  if (_contiguous_assignments_only && num_neighborless)
    mooseWarning("Several subdomains were created for neighborless elements: " +
                 std::to_string(num_neighborless));

  // subdomain assignments have changed
  mesh->unset_is_prepared();
  return dynamic_pointer_cast<MeshBase>(mesh);
}

void
SurfaceSubdomainsFromAllNormalsGenerator::actOnElem(Elem * const elem,
                                                    const Point &,
                                                    const subdomain_id_type & sub_id,
                                                    MeshBase &)
{
  elem->subdomain_id() = sub_id;
}
