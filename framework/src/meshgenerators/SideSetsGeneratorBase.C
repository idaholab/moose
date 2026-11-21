//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#include "MeshTraversingUtils.h"

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

  // To know the dimension of the mesh. Only the steps that are marked as needed are performed, so
  // that a chain of sideset generators does not rebuild the neighbors of the whole mesh each time.
  if (!mesh.is_prepared())
  {
    // After a sideset generator, only the cached boundary id sets are usually stale. Regenerating
    // them is all that is needed; complete_preparation() would also count the unpartitioned
    // elements and nodes of the whole mesh just to set a flag that is already set.
    auto only_boundary_id_sets_stale = mesh.preparation();
    only_boundary_id_sets_stale.has_boundary_id_sets = true;
    if (only_boundary_id_sets_stale)
    {
      mesh.get_boundary_info().regenerate_id_sets();
      mooseAssert(mesh.is_prepared(), "Regenerating the boundary id sets should prepare the mesh");
    }
    else
    {
      // Sideset generators do not change element or node ids, so the renumbering that
      // complete_preparation() would otherwise always do at its end is not needed. Renumbering
      // that is needed after elements or nodes were removed is still done, as it is tracked by
      // flags.
      const bool allow_renumbering = mesh.allow_renumbering();
      mesh.allow_renumbering(false);
      mesh.complete_preparation();
      mesh.allow_renumbering(allow_renumbering);
    }
  }
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

  // Check that the subdomains exist in the mesh and get their ids. The mesh is prepared at this
  // point, so its cached subdomain ids (across all processors) are current and no loop over the
  // elements is needed.
  const auto get_subdomain_ids = [&](const std::string & param_name)
  {
    const auto & mesh_subdomain_ids = mesh.get_mesh_subdomains();
    const auto subdomains = getParam<std::vector<SubdomainName>>(param_name);
    for (const auto & name : subdomains)
    {
      const auto id = MooseMeshUtils::getSubdomainID(name, mesh);
      if (id == Moose::INVALID_BLOCK_ID || !mesh_subdomain_ids.count(id))
        paramError(param_name, "The block '", name, "' was not found in the mesh");
    }
    return MooseMeshUtils::getSubdomainIDs(mesh, subdomains);
  };

  if (parameters().isParamValid("included_subdomains"))
    _included_subdomain_ids = get_subdomain_ids("included_subdomains");

  if (parameters().isParamValid("included_neighbors"))
    _included_neighbor_subdomain_ids = get_subdomain_ids("included_neighbors");

  // We need neighbors per #15823. complete_preparation() above finds them only if they are not
  // already known, as tracked by the mesh's preparation state.
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
  if (_check_subdomains &&
      !MeshTraversingUtils::elementSubdomainIdInList(elem, _included_subdomain_ids))
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
SideSetsGeneratorBase::elementSideInIncludedBoundaries(const Elem * const elem,
                                                       const unsigned int side,
                                                       const MeshBase & mesh) const
{
  // Query the boundary ids of the side once; has_boundary_id() does so for every id
  std::vector<boundary_id_type> side_boundary_ids;
  mesh.get_boundary_info().boundary_ids(elem, side, side_boundary_ids);
  for (const auto & bid : _included_boundary_ids)
    if (std::find(side_boundary_ids.begin(), side_boundary_ids.end(), bid) !=
        side_boundary_ids.end())
      return true;
  return false;
}

bool
SideSetsGeneratorBase::elementSideInExcludedBoundaries(const Elem * const elem,
                                                       const unsigned int side,
                                                       const MeshBase & mesh) const
{
  // Query the boundary ids of the side once; has_boundary_id() does so for every id
  std::vector<boundary_id_type> side_boundary_ids;
  mesh.get_boundary_info().boundary_ids(elem, side, side_boundary_ids);
  for (const auto bid : _excluded_boundary_ids)
    if (std::find(side_boundary_ids.begin(), side_boundary_ids.end(), bid) !=
        side_boundary_ids.end())
      return true;
  return false;
}

bool
SideSetsGeneratorBase::elemSideSatisfiesNonNormalRequirements(const Elem * const elem,
                                                              const unsigned int side,
                                                              const MeshBase & mesh) const
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
    if (!(neighbor && MeshTraversingUtils::elementSubdomainIdInList(
                          neighbor, _included_neighbor_subdomain_ids)))
      return false;
  }

  return true;
}

bool
SideSetsGeneratorBase::elemSideSatisfiesRequirements(const Elem * const elem,
                                                     const unsigned int side,
                                                     const MeshBase & mesh,
                                                     const Point & desired_normal,
                                                     const Point & face_normal)
{
  if (!elemSideSatisfiesNonNormalRequirements(elem, side, mesh))
    return false;

  if (_using_normal &&
      !MeshTraversingUtils::normalsWithinTol(desired_normal, face_normal, _normal_tol))
    return false;

  return true;
}
