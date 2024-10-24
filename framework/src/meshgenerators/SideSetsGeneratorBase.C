//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideSetsGeneratorBase.h"
#include "Parser.h"
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
SideSetsGeneratorBase::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addRequiredParam<std::vector<BoundaryName>>(
      "new_boundary", "The list of boundary names to create on the supplied subdomain");
  params.addParam<bool>("fixed_normal",
                        false,
                        "This Boolean determines whether we fix our normal "
                        "or allow it to vary to \"paint\" around curves");

  params.addParam<bool>("replace",
                        false,
                        "If true, replace the old sidesets. If false, the current sidesets (if "
                        "any) will be preserved.");

  params.addParam<std::vector<BoundaryName>>(
      "included_boundaries",
      "A set of boundary names or ids whose sides will be included in the new sidesets.  A side "
      "is only added if it also belongs to one of these boundaries.");
  params.addParam<std::vector<BoundaryName>>(
      "excluded_boundaries",
      "A set of boundary names or ids whose sides will be excluded from the new sidesets.  A side "
      "is only added if does not belong to any of these boundaries.");
  params.addParam<std::vector<SubdomainName>>(
      "included_subdomains",
      "A set of subdomain names or ids whose sides will be included in the new sidesets. A side "
      "is only added if the subdomain id of the corresponding element is in this set.");
  params.addParam<std::vector<SubdomainName>>("included_neighbors",
                                              "A set of neighboring subdomain names or ids. A face "
                                              "is only added if the subdomain id of the "
                                              "neighbor is in this set");
  params.addParam<bool>(
      "include_only_external_sides",
      false,
      "Whether to only include external sides when considering sides to add to the sideset");

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
  params.addParam<Real>("variance", "The variance allowed when comparing normals");
  params.deprecateParam("variance", "normal_tol", "4/01/2025");

  // Sideset restriction param group
  params.addParamNamesToGroup(
      "included_boundaries excluded_boundaries included_subdomains included_neighbors "
      "include_only_external_sides normal normal_tol",
      "Sideset restrictions");

  return params;
}

SideSetsGeneratorBase::SideSetsGeneratorBase(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _boundary_names(std::vector<BoundaryName>()),
    _fixed_normal(getParam<bool>("fixed_normal")),
    _replace(getParam<bool>("replace")),
    _check_included_boundaries(isParamValid("included_boundaries")),
    _check_excluded_boundaries(isParamValid("excluded_boundaries")),
    _check_subdomains(isParamValid("included_subdomains")),
    _check_neighbor_subdomains(isParamValid("included_neighbors")),
    _included_boundary_ids(std::vector<boundary_id_type>()),
    _excluded_boundary_ids(std::vector<boundary_id_type>()),
    _included_subdomain_ids(std::vector<subdomain_id_type>()),
    _included_neighbor_subdomain_ids(std::vector<subdomain_id_type>()),
    _include_only_external_sides(getParam<bool>("include_only_external_sides")),
    _using_normal(isParamSetByUser("normal")),
    _normal(_using_normal ? Point(getParam<Point>("normal") / getParam<Point>("normal").norm())
                          : getParam<Point>("normal")),
    _normal_tol(getParam<Real>("normal_tol"))
{
  if (isParamValid("new_boundary"))
    _boundary_names = getParam<std::vector<BoundaryName>>("new_boundary");
}

SideSetsGeneratorBase::~SideSetsGeneratorBase() {}

void
SideSetsGeneratorBase::setup(MeshBase & mesh)
{
  mooseAssert(_fe_face == nullptr, "FE Face has already been initialized");

  // To know the dimension of the mesh
  if (!mesh.is_prepared())
    mesh.prepare_for_use();
  const auto dim = mesh.mesh_dimension();

  // Setup the FE Object so we can calculate normals
  libMesh::FEType fe_type(Utility::string_to_enum<Order>("CONSTANT"),
                          Utility::string_to_enum<libMesh::FEFamily>("MONOMIAL"));
  _fe_face = libMesh::FEBase::build(dim, fe_type);
  _qface = std::make_unique<libMesh::QGauss>(dim - 1, FIRST);
  _fe_face->attach_quadrature_rule(_qface.get());
  // Must always pre-request quantities you want to compute
  _fe_face->get_normals();

  // Handle incompatible parameters
  if (_include_only_external_sides && _check_neighbor_subdomains)
    paramError("include_only_external_sides", "External sides dont have neighbors");

  if (_check_included_boundaries)
  {
    const auto & included_boundaries = getParam<std::vector<BoundaryName>>("included_boundaries");
    for (const auto & boundary_name : _boundary_names)
      if (std::find(included_boundaries.begin(), included_boundaries.end(), boundary_name) !=
          included_boundaries.end())
        paramError(
            "new_boundary",
            "A boundary cannot be both the new boundary and be included in the list of included "
            "boundaries. If you are trying to restrict an existing boundary, you must use a "
            "different name for 'new_boundary', delete the old boundary, and then rename the "
            "new boundary to the old boundary.");

    _included_boundary_ids = MooseMeshUtils::getBoundaryIDs(mesh, included_boundaries, false);

    // Check that the included boundary ids/names exist in the mesh
    for (const auto i : index_range(_included_boundary_ids))
      if (_included_boundary_ids[i] == Moose::INVALID_BOUNDARY_ID)
        paramError("included_boundaries",
                   "The boundary '",
                   included_boundaries[i],
                   "' was not found within the mesh");
  }

  if (_check_excluded_boundaries)
  {
    const auto & excluded_boundaries = getParam<std::vector<BoundaryName>>("excluded_boundaries");
    for (const auto & boundary_name : _boundary_names)
      if (std::find(excluded_boundaries.begin(), excluded_boundaries.end(), boundary_name) !=
          excluded_boundaries.end())
        paramError(
            "new_boundary",
            "A boundary cannot be both the new boundary and be excluded in the list of excluded "
            "boundaries.");
    _excluded_boundary_ids = MooseMeshUtils::getBoundaryIDs(mesh, excluded_boundaries, false);

    // Check that the excluded boundary ids/names exist in the mesh
    for (const auto i : index_range(_excluded_boundary_ids))
      if (_excluded_boundary_ids[i] == Moose::INVALID_BOUNDARY_ID)
        paramError("excluded_boundaries",
                   "The boundary '",
                   excluded_boundaries[i],
                   "' was not found within the mesh");

    if (_check_included_boundaries)
    {
      // Check that included and excluded boundary lists do not overlap
      for (const auto & boundary_id : _included_boundary_ids)
        if (std::find(_excluded_boundary_ids.begin(), _excluded_boundary_ids.end(), boundary_id) !=
            _excluded_boundary_ids.end())
          paramError("excluded_boundaries",
                     "'included_boundaries' and 'excluded_boundaries' lists should not overlap");
    }
  }

  // Get the boundary ids from the names
  if (parameters().isParamValid("included_subdomains"))
  {
    // check that the subdomains exist in the mesh
    const auto subdomains = getParam<std::vector<SubdomainName>>("included_subdomains");
    for (const auto & name : subdomains)
      if (!MooseMeshUtils::hasSubdomainName(mesh, name))
        paramError("included_subdomains", "The block '", name, "' was not found in the mesh");

    _included_subdomain_ids = MooseMeshUtils::getSubdomainIDs(mesh, subdomains);
  }

  if (parameters().isParamValid("included_neighbors"))
  {
    // check that the subdomains exist in the mesh
    const auto subdomains = getParam<std::vector<SubdomainName>>("included_neighbors");
    for (const auto & name : subdomains)
      if (!MooseMeshUtils::hasSubdomainName(mesh, name))
        paramError("included_neighbors", "The block '", name, "' was not found in the mesh");

    _included_neighbor_subdomain_ids = MooseMeshUtils::getSubdomainIDs(mesh, subdomains);
  }

  // We will want to Change the below code when we have more fine-grained control over advertising
  // what we need and how we satisfy those needs. For now we know we need to have neighbors per
  // #15823...and we do have an explicit `find_neighbors` call...but we don't have a
  // `neighbors_found` API and it seems off to do:
  //
  // if (!mesh.is_prepared())
  //   mesh.find_neighbors()
}

void
SideSetsGeneratorBase::finalize()
{
  _qface.reset();
  _fe_face.reset();
}

void
SideSetsGeneratorBase::flood(const Elem * elem,
                             const Point & normal,
                             const boundary_id_type & side_id,
                             MeshBase & mesh)
{
  if (elem == nullptr || elem == remote_elem ||
      (_visited[side_id].find(elem) != _visited[side_id].end()))
    return;

  // Skip if element is not in specified subdomains
  if (_check_subdomains && !elementSubdomainIdInList(elem, _included_subdomain_ids))
    return;

  _visited[side_id].insert(elem);

  // Request to compute normal vectors
  const std::vector<Point> & face_normals = _fe_face->get_normals();

  for (const auto side : make_range(elem->n_sides()))
  {

    _fe_face->reinit(elem, side);
    // We'll just use the normal of the first qp
    const Point face_normal = face_normals[0];

    if (!elemSideSatisfiesRequirements(elem, side, mesh, normal, face_normal))
      continue;

    if (_replace)
      mesh.get_boundary_info().remove_side(elem, side);

    mesh.get_boundary_info().add_side(elem, side, side_id);
    for (const auto neighbor : make_range(elem->n_sides()))
    {
      // Flood to the neighboring elements using the current matching side normal from this
      // element.
      // This will allow us to tolerate small changes in the normals so we can "paint" around a
      // curve.
      flood(elem->neighbor_ptr(neighbor), _fixed_normal ? normal : face_normal, side_id, mesh);
    }
  }
}

bool
SideSetsGeneratorBase::normalsWithinTol(const Point & normal_1,
                                        const Point & normal_2,
                                        const Real & tol) const
{
  return (1.0 - normal_1 * normal_2) <= tol;
}

bool
SideSetsGeneratorBase::elementSubdomainIdInList(
    const Elem * const elem, const std::vector<subdomain_id_type> & subdomain_id_list) const
{
  subdomain_id_type curr_subdomain = elem->subdomain_id();
  return std::find(subdomain_id_list.begin(), subdomain_id_list.end(), curr_subdomain) !=
         subdomain_id_list.end();
}

bool
SideSetsGeneratorBase::elementSideInIncludedBoundaries(const Elem * const elem,
                                                       const unsigned int side,
                                                       const MeshBase & mesh) const
{
  for (const auto & bid : _included_boundary_ids)
    if (mesh.get_boundary_info().has_boundary_id(elem, side, bid))
      return true;
  return false;
}

bool
SideSetsGeneratorBase::elementSideInExcludedBoundaries(const Elem * const elem,
                                                       const unsigned int side,
                                                       const MeshBase & mesh) const
{
  for (const auto bid : _excluded_boundary_ids)
    if (mesh.get_boundary_info().has_boundary_id(elem, side, bid))
      return true;
  return false;
}

bool
SideSetsGeneratorBase::elemSideSatisfiesRequirements(const Elem * const elem,
                                                     const unsigned int side,
                                                     const MeshBase & mesh,
                                                     const Point & desired_normal,
                                                     const Point & face_normal)
{
  // Skip if side has neighbor and we only want external sides
  if ((elem->neighbor_ptr(side) && _include_only_external_sides))
    return false;

  // Skip if side is not part of included boundaries
  if (_check_included_boundaries && !elementSideInIncludedBoundaries(elem, side, mesh))
    return false;
  // Skip if side is part of excluded boundaries
  if (_check_excluded_boundaries && elementSideInExcludedBoundaries(elem, side, mesh))
    return false;

  // Skip if element does not have neighbor in specified subdomains
  if (_check_neighbor_subdomains)
  {
    const Elem * const neighbor = elem->neighbor_ptr(side);
    // if the neighbor does not exist, then skip this face; we only add sidesets
    // between existing elems if _check_neighbor_subdomains is true
    if (!(neighbor && elementSubdomainIdInList(neighbor, _included_neighbor_subdomain_ids)))
      return false;
  }

  if (_using_normal && !normalsWithinTol(desired_normal, face_normal, _normal_tol))
    return false;

  return true;
}
