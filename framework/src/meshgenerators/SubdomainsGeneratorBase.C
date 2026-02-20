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
#include "MooseMesh.h"
#include "MooseMeshUtils.h"

#include "libmesh/mesh_generation.h"
#include "libmesh/mesh.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/point_locator_base.h"
#include "libmesh/elem.h"
#include "libmesh/remote_elem.h"

InputParameters
SubdomainsGeneratorBase::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addRequiredParam<std::vector<SubdomainName>>(
      "new_subdomain", "The list of subdomain names to create on the supplied subdomain");
  params.addParam<std::vector<SubdomainName>>(
      "included_subdomains",
      "A set of subdomain names or ids an element has to be previously a part of to be moved into "
      "a new subdomain.");

  // Useful for painting subdomains over 2D surface elements
  params.addParam<Point>("normal",
                         Point(),
                         "If supplied, only faces with normal equal to this, up to "
                         "normal_tol, will be added to the sidesets specified");
  params.addRangeCheckedParam<Real>("normal_tol",
                                    0.1,
                                    "normal_tol>=0 & normal_tol<=2",
                                    "If normal is supplied then faces are "
                                    "only added if face_normal.normal_hat >= "
                                    "1 - normal_tol, where normal_hat = "
                                    "normal/|normal|");
  params.addParam<bool>("fixed_normal",
                        false,
                        "Whether to move the normal vector as we paint the geometry, or keep it "
                        "fixed from the first element we started painting with");
  params.addRangeCheckedParam<std::vector<Real>>(
      "max_subdomain_size_centroids",
      "max_subdomain_size_centroids >= 0",
      "Maximum distance between element centroids (vertex average approximation) "
      "in each 'included_subdomain'. 0 means do not apply a distance, a single value in the vector "
      "is applied to all subdomains");

  // Flood parameters
  // NOTE: this can 'cut' paths to re-grouping elements. It is a heuristic and won't always improve
  // things
  params.addParam<bool>("flood_elements_once", false, "Whether to consider elements only once");
  params.addParam<bool>("check_painted_neighbor_normals",
                        false,
                        "When examining the normal of a 2D element and comparing to the 'painting' "
                        "normal, also check if the element neighbors in the 'painted' subdomain "
                        "have a closer normal and accept the element into the new subdomain if so");

  return params;
}

SubdomainsGeneratorBase::SubdomainsGeneratorBase(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _subdomain_names(std::vector<SubdomainName>()),
    _check_subdomains(isParamValid("included_subdomains")),
    _included_subdomain_ids(std::vector<subdomain_id_type>()),
    _using_normal(isParamSetByUser("normal") || isParamValid("_using_normal")),
    _normal(isParamSetByUser("normal")
                ? Point(getParam<Point>("normal") / getParam<Point>("normal").norm())
                : getParam<Point>("normal")),
    _normal_tol(getParam<Real>("normal_tol")),
    _fixed_normal(getParam<bool>("fixed_normal")),
    _has_max_distance_criterion(isParamSetByUser("max_subdomain_size_centroids")),
    _flood_only_once(getParam<bool>("flood_elements_once")),
    _check_painted_neighor_normals(getParam<bool>("check_painted_neighbor_normals"))
{
  if (isParamValid("new_subdomain"))
    _subdomain_names = getParam<std::vector<SubdomainName>>("new_subdomain");
}

void
SubdomainsGeneratorBase::setup(MeshBase & mesh)
{
  // To know the dimension of the mesh
  if (!mesh.is_prepared())
    mesh.prepare_for_use();

  // Get the subdomain ids from the names
  if (parameters().isParamValid("included_subdomains"))
  {
    // check that the subdomains exist in the mesh
    const auto subdomains = getParam<std::vector<SubdomainName>>("included_subdomains");
    for (const auto & name : subdomains)
      if (!MooseMeshUtils::hasSubdomainName(mesh, name))
        paramError("included_subdomains", "The block '", name, "' was not found in the mesh");

    _included_subdomain_ids = MooseMeshUtils::getSubdomainIDs(mesh, subdomains);
  }

  // Set up the max elem-to-elem distance map
  if (_has_max_distance_criterion)
  {
    const auto & max_dists = getParam<std::vector<Real>>("max_subdomain_size_centroids");
    if (max_dists.size() != _included_subdomain_ids.size() && max_dists.size() != 1)
      paramError("max_subdomain_size_centroids",
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
}

void
SubdomainsGeneratorBase::flood(Elem * const elem,
                               const Point & base_normal,
                               const Elem & starting_elem,
                               const subdomain_id_type & sub_id)
{
  if (elem == nullptr || elem == remote_elem ||
      (_visited[sub_id].find(elem) != _visited[sub_id].end()))
    return;
  if (_flood_only_once && _visited_once.count(elem))
    return;

  // Skip if element is not in specified subdomains
  if (_check_subdomains && !elementSubdomainIdInList(elem, _included_subdomain_ids))
    return;

  _visited[sub_id].insert(elem);
  const auto elem_normal = get2DElemNormal(elem);

  bool criterion_met = false;
  if (elementSatisfiesRequirements(elem, base_normal, starting_elem, elem_normal))
    criterion_met = true;
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

  elem->subdomain_id() = sub_id;

  // We don't want to remove the element from consideration too early
  _visited_once.insert(elem);

  for (const auto neighbor : make_range(elem->n_sides()))
  {
    // Flood to the neighboring elements using the current matching side normal from this
    // element.
    flood(elem->neighbor_ptr(neighbor),
          _fixed_normal ? base_normal : elem_normal,
          starting_elem,
          sub_id);
  }
}

bool
SubdomainsGeneratorBase::normalsWithinTol(const Point & normal_1,
                                          const Point & normal_2,
                                          const Real & tol) const
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
  // Skip if element is not in specified subdomains
  // NOTE: we are checking this twice when calling from flood()
  if (_check_subdomains && !elementSubdomainIdInList(elem, _included_subdomain_ids))
    return false;

  if (_using_normal && !normalsWithinTol(desired_normal, face_normal, _normal_tol))
    return false;

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
    mooseWarning("Colinear nodes on elements, using 0 normal");
    return elem_normal;
  }
  return elem_normal.unit();
}
