//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SubdomainsGeneratorBase.h"
#include "InputParameters.h"
#include "MooseMeshUtils.h"

#include "libmesh/mesh_generation.h"
#include "libmesh/mesh.h"
#include "libmesh/elem.h"
#include "libmesh/remote_elem.h"
#include "libmesh/string_to_enum.h"

InputParameters
SubdomainsGeneratorBase::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  // NOTE: don't forget to suppress this if not using this base class to create subdomains
  params.addRequiredParam<std::vector<SubdomainName>>(
      "new_subdomain", "The list of subdomain names to create on the supplied subdomain");
  params.addParam<std::vector<SubdomainName>>(
      "included_subdomains",
      "A set of subdomain names or ids an element has to be previously a part of to be considered "
      "for modification.");

  // Painting/flooding parameters
  // Using normals
  params.addParam<Point>("normal",
                         Point(),
                         "If supplied, only surface (2D) elements with normal equal to this, up to "
                         "normal_tol, will be considered for modification");
  params.addRangeCheckedParam<Real>("normal_tol",
                                    0.1,
                                    "normal_tol>=0 & normal_tol<=2",
                                    "Surface elements are "
                                    "only added if face_normal.normal_hat >= "
                                    "1 - normal_tol, where normal_hat = "
                                    "normal/|normal|. The normal can the normal specified by the "
                                    "parameter or by a specific mesh generator.");
  params.addRangeCheckedParam<Real>("flipped_normal_tol",
                                    0.1,
                                    "flipped_normal_tol>=0 & flipped_normal_tol<=2",
                                    "If 'allow_normal_flips', surface elements are "
                                    "also added if -1 * face_normal.normal_hat >= "
                                    "1 - normal_tol, where normal_hat = "
                                    "normal/|normal|");
  params.addParam<bool>(
      "fixed_normal",
      false,
      "Whether to move the normal vector as we paint/flood the geometry, or keep it "
      "fixed from the first element we started painting with");
  params.addParam<bool>(
      "allow_normal_flips",
      false,
      "Whether to allow for elements to be considered "
      "when their normal is flipped with regard to the neighbor element we are painting from. A "
      "specific tolerance may be specified for these flipped normals.");
  params.addParam<bool>("check_painted_neighbor_normals",
                        false,
                        "When examining the normal of a 2D element and comparing to the 'painting' "
                        "normal, also check if the element neighbors in the 'painted' subdomain "
                        "have a closer normal and accept the element into the new subdomain if so");

  // Other painting / flooding parameters
  params.addParam<std::vector<Real>>(
      "max_paint_size_centroids",
      "Maximum distance between element centroids (using a vertex average approximation) "
      "when painting/flooding the geometry. 0 means not applying a distance constraint, a single "
      "value in the vector is applied to all elements, multiple values can be specified for each "
      "'included_subdomains'");
  params.addParam<bool>(
      "flood_elements_once",
      false,
      "Whether to consider elements only once when painting/flooding the geometry");

  params.addParamNamesToGroup("normal normal_tol allow_normal_flips flipped_normal_tol "
                              "fixed_normal check_painted_neighbor_normals",
                              "Flooding using surface element normals");
  params.addParamNamesToGroup("max_paint_size_centroids flood_elements_once", "Other flooding");
  return params;
}

SubdomainsGeneratorBase::SubdomainsGeneratorBase(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _subdomain_names(isParamValid("new_subdomain")
                         ? getParam<std::vector<SubdomainName>>("new_subdomain")
                         : std::vector<SubdomainName>()),
    _check_subdomains(isParamValid("included_subdomains")),
    _included_subdomain_ids(std::vector<subdomain_id_type>()),
    _using_normal(isParamSetByUser("normal") || isParamValid("_using_normal")),
    _normal(isParamSetByUser("normal")
                ? Point(getParam<Point>("normal") / getParam<Point>("normal").norm())
                : getParam<Point>("normal")),
    _normal_tol(getParam<Real>("normal_tol")),
    _flipped_normal_tol(getParam<Real>("flipped_normal_tol")),
    _fixed_normal(getParam<bool>("fixed_normal")),
    _allow_normal_flips(getParam<bool>("allow_normal_flips")),
    _has_max_distance_criterion(isParamSetByUser("max_paint_size_centroids")),
    _flood_only_once(getParam<bool>("flood_elements_once")),
    _check_painted_neighor_normals(getParam<bool>("check_painted_neighbor_normals"))
{
}

void
SubdomainsGeneratorBase::setup(MeshBase & mesh)
{
  // To know the dimension of the mesh
  if (!mesh.is_prepared())
    mesh.prepare_for_use();

  // Get the subdomain ids from the names
  if (isParamValid("included_subdomains"))
  {
    // check that the subdomains exist in the mesh
    const auto & subdomains = getParam<std::vector<SubdomainName>>("included_subdomains");
    for (const auto & name : subdomains)
      if (!MooseMeshUtils::hasSubdomainName(mesh, name))
        paramError("included_subdomains", "The block '", name, "' was not found in the mesh");

    _included_subdomain_ids = MooseMeshUtils::getSubdomainIDs(mesh, subdomains);
  }

  // Set up the max elem-to-elem distance map
  if (_has_max_distance_criterion)
  {
    const auto & max_dists = getParam<std::vector<Real>>("max_paint_size_centroids");
    if (max_dists.size() != _included_subdomain_ids.size() && max_dists.size() != 1)
      paramError("max_paint_size_centroids",
                 "Maximum distance should be specified uniformly for all subdomains (1 value) or a "
                 "value for each 'included_subdomains'");

    if (_included_subdomain_ids.size())
      for (const auto i : index_range(_included_subdomain_ids))
        // Single value is applied for all subdomains
        // 0 is translated to a very big number which therefore won't impose the criterion
        _max_elem_distance[_included_subdomain_ids[i]] =
            (max_dists.size() == 1)
                ? max_dists[0]
                : (max_dists[i] > 0 ? max_dists[i]
                                    : std::pow(std::numeric_limits<Real>::max(), 0.3));
    else
      _max_elem_distance[static_cast<subdomain_id_type>(-1)] = max_dists[0];
  }

  // Clear the maps used to count visits
  _visited.clear();
  _acted_upon_once.clear();
}

void
SubdomainsGeneratorBase::flood(Elem * const elem,
                               const Point & base_normal,
                               const Elem & starting_elem,
                               const subdomain_id_type & sub_id,
                               MeshBase & mesh)
{
  // Avoid re-considering the same elements
  if (elem == nullptr || elem == remote_elem ||
      (_visited[sub_id].find(elem) != _visited[sub_id].end()))
    return;
  if (_flood_only_once && _acted_upon_once.count(elem))
    return;

  // Skip if element is not in specified subdomains
  if (_check_subdomains && !elementSubdomainIdInList(elem, _included_subdomain_ids))
    return;

  _visited[sub_id].insert(elem);
  auto elem_normal = get2DElemNormal(elem);

  bool criterion_met = false;
  if (elementSatisfiesRequirements(elem, base_normal, starting_elem, elem_normal))
    criterion_met = true;
  // Additional heuristic based on neighbor normal vs element normal
  else if (_check_painted_neighor_normals)
  {
    // Try to flood from each side with the same subdomain
    for (const auto neighbor : make_range(elem->n_sides()))
      if (elem->neighbor_ptr(neighbor) &&
          (elem->neighbor_ptr(neighbor)->subdomain_id() == sub_id) &&
          elementSatisfiesRequirements(
              elem, get2DElemNormal(elem->neighbor_ptr(neighbor)), starting_elem, elem_normal))
        criterion_met = true;
  }

  if (!criterion_met)
    return;

  // Modify the subdomain
  // NOTE: if we want to perform a different action, we should have a callback here instead
  elem->subdomain_id() = sub_id;

  // We don't want to remove the element from consideration too early
  _acted_upon_once.insert(elem);

  // Flip the element if needed
  // NOTE: we have already passed "elementSatisfiesRequirements" here
  if (_allow_normal_flips && base_normal * elem_normal < 0)
  {
    elem_normal *= -1;
    BoundaryInfo & boundary_info = mesh.get_boundary_info();

    elem->flip(&boundary_info);
  }

  for (const auto neighbor : make_range(elem->n_sides()))
  {
    // Flood to the neighboring elements
    flood(elem->neighbor_ptr(neighbor),
          _fixed_normal ? base_normal : elem_normal,
          starting_elem,
          sub_id,
          mesh);
  }
}

bool
SubdomainsGeneratorBase::normalsWithinTol(const Point & normal_1,
                                          const Point & normal_2,
                                          const Real tol) const
{
  return (1.0 - normal_1 * normal_2) <= tol;
}

bool
SubdomainsGeneratorBase::elementSubdomainIdInList(
    const Elem * const elem, const std::vector<subdomain_id_type> & subdomain_id_list) const
{
  subdomain_id_type curr_subdomain = elem->subdomain_id();
  return std::find(subdomain_id_list.begin(), subdomain_id_list.end(), curr_subdomain) !=
         subdomain_id_list.end();
}

bool
SubdomainsGeneratorBase::elementSatisfiesRequirements(const Elem * const elem,
                                                      const Point & desired_normal,
                                                      const Elem & base_elem,
                                                      const Point & face_normal) const
{
  // False if element is not in specified subdomains
  if (_check_subdomains && !elementSubdomainIdInList(elem, _included_subdomain_ids))
    return false;

  // False if normal does not meet criteria
  if (_using_normal && (!normalsWithinTol(desired_normal, face_normal, _normal_tol) &&
                        (!_allow_normal_flips ||
                         !normalsWithinTol(desired_normal, -face_normal, _flipped_normal_tol))))
    return false;

  // False if exceeding the patch size
  if (_has_max_distance_criterion)
  {
    // The subdomain from which the element to paint over comes from is used to find the limitation
    // on the radius, which will effectively be applied onto the new subdomains (to which base_elem
    // already belongs)
    const auto max_dsq =
        _check_subdomains
            ? MathUtils::pow(libmesh_map_find(_max_elem_distance, elem->subdomain_id()), 2)
            : MathUtils::pow(
                  libmesh_map_find(_max_elem_distance, static_cast<subdomain_id_type>(-1)), 2);
    if ((elem->vertex_average() - base_elem.vertex_average()).norm_sq() > max_dsq)
      return false;
  }

  return true;
}

Point
SubdomainsGeneratorBase::get2DElemNormal(const Elem * const elem) const
{
  mooseAssert(elem->dim() == 2, "Should be a 2D element");
  mooseAssert(elem->default_order() == FIRST, "Should be a first order element");

  const auto & p1 = elem->point(0);
  const auto & p2 = elem->point(1);
  const auto & p3 = elem->point(2);
  auto elem_normal = (p1 - p2).cross(p1 - p3);
  if (elem_normal.norm_sq() == 0)
  {
    mooseWarning("Colinear nodes on element " + std::to_string(elem->id()) + ", using 0 normal");
    return elem_normal;
  }
  return elem_normal.unit();
}
