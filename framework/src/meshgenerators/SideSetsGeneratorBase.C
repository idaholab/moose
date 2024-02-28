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

#include "libmesh/mesh_generation.h"
#include "libmesh/mesh.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/point_locator_base.h"
#include "libmesh/fe_base.h"
#include "libmesh/elem.h"

InputParameters
SideSetsGeneratorBase::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
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
      "A set of boundary names or ids whose sides will be included in the new sidesets");
  params.addParam<std::vector<SubdomainName>>(
      "included_subdomains",
      "A set of subdomain names or ids whose sides will be included in the new sidesets");
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
  params.addDeprecatedParam<Real>("variance",
                                  "The variance allowed when comparing normals",
                                  "Deprecated, use 'normal_tol' instead");
  params.deprecateParam("variance", "normal_tol", "4/01/2024");

  // Sideset restriction param group
  params.addParamNamesToGroup("included_boundaries included_subdomains included_neighbors "
                              "include_only_external_sides normal normal_tol",
                              "Sideset restrictions");

  return params;
}

SideSetsGeneratorBase::SideSetsGeneratorBase(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _fixed_normal(getParam<bool>("fixed_normal")),
    _replace(getParam<bool>("replace")),
    _check_boundaries(isParamValid("included_boundaries")),
    _check_subdomains(isParamValid("included_subdomain_ids") ||
                      isParamValid("included_subdomains")),
    _check_neighbor_subdomains(isParamValid("included_neighbor_ids") ||
                               isParamValid("included_neighbors")),
    _included_ids(std::vector<SubdomainID>()),
    _included_neighbor_ids(std::vector<SubdomainID>()),
    _include_only_external_sides(getParam<bool>("include_only_external_sides")),
    _normal(getParam<Point>("normal")),
    _using_normal(isParamSetByUser("normal")),
    _normal_tol(getParam<Real>("normal_tol"))
{
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
  FEType fe_type(Utility::string_to_enum<Order>("CONSTANT"),
                 Utility::string_to_enum<FEFamily>("MONOMIAL"));
  _fe_face = FEBase::build(dim, fe_type);
  _qface = std::make_unique<QGauss>(dim - 1, FIRST);
  _fe_face->attach_quadrature_rule(_qface.get());

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
                             Point normal,
                             boundary_id_type side_id,
                             MeshBase & mesh)
{
  if (elem == nullptr || (_visited[side_id].find(elem) != _visited[side_id].end()))
    return;

  _visited[side_id].insert(elem);
  for (const auto side : make_range(elem->n_sides()))
  {
    if (elem->neighbor_ptr(side))
      continue;

    _fe_face->reinit(elem, side);
    const std::vector<Point> normals = _fe_face->get_normals();

    // We'll just use the normal of the first qp
    if (normalsWithinTol(normal, normals[0]))
    {
      if (_replace)
        mesh.get_boundary_info().remove_side(elem, side);

      mesh.get_boundary_info().add_side(elem, side, side_id);
      for (const auto neighbor : make_range(elem->n_sides()))
      {
        // Flood to the neighboring elements using the current matching side normal from this
        // element.
        // This will allow us to tolerate small changes in the normals so we can "paint" around a
        // curve.
        flood(elem->neighbor_ptr(neighbor), _fixed_normal ? normal : normals[0], side_id, mesh);
      }
    }
  }
}

bool
SideSetsGeneratorBase::normalsWithinTol(const Point & normal_1, const Point & normal_2) const
{
  return std::abs(1.0 - normal_1 * normal_2) <= _normal_tol;
}
