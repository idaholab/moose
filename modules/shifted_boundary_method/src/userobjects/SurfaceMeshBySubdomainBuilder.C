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
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription("Builds subdomain-grouped centroids, element IDs, and boundary "
                             "elements from a given surface mesh.");

  params.addRequiredParam<std::string>(
      "surface_mesh",
      "The name of the surface mesh saved via the MeshGenerator's save_mesh_as parameter.");

  params.addParam<int>(
      "leaf_max_size", 10, "Maximum number of points allowed in a leaf node of the KDTree.");

  params.addParam<bool>(
      "check_watertightness",
      false,
      "Check if the mesh is watertight. If false, the mesh may not be suitable for In-Out tests.");

  params.addParam<bool>("check_replicated",
                        true,
                        "Check if the mesh is replicated (not distributed). If false, the mesh may "
                        "not be suitable for In-Out tests.");

  return params;
}

SurfaceMeshBySubdomainBuilder::SurfaceMeshBySubdomainBuilder(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _leaf_max_size(getParam<int>("leaf_max_size")),
    _bnd_mesh_name(getParam<std::string>("surface_mesh")),
    _check_watertightness(getParam<bool>("check_watertightness")),
    _dim_embedding_mesh(_fe_problem.mesh().dimension()),
    _check_replicated(getParam<bool>("check_replicated"))
{
}

void
SurfaceMeshBySubdomainBuilder::initialSetup()
{
  auto & mesh_generator_system = _app.getMeshGeneratorSystem();
  _mesh = mesh_generator_system.getSavedMesh(_bnd_mesh_name);

  const auto expected_dim_embedding_mesh = _mesh->mesh_dimension() + 1;

  if (!_mesh->is_replicated() && _check_replicated)
    mooseError("The mesh is distributed. Please use a serial mesh for In-Out testing.");

  if (_dim_embedding_mesh != expected_dim_embedding_mesh)
    mooseError("Mismatch between embedding mesh dimension (",
               _dim_embedding_mesh,
               ") and surface mesh dimension + 1 (",
               expected_dim_embedding_mesh,
               ").");

  buildSubdomainGroupedData();

  if (_check_watertightness)
  {
    bool watertight = checkWatertightness();
    if (!watertight)
      mooseInfo("The mesh is not watertight. It may not be suitable for In-Out tests.");
    else
      mooseInfo("The mesh is watertight. It is suitable for In-Out tests.");
  }
}

void
SurfaceMeshBySubdomainBuilder::buildSubdomainGroupedData()
{
  for (auto elem_it = _mesh->active_elements_begin(); elem_it != _mesh->active_elements_end();
       ++elem_it)
  {
    const Elem * elem = *elem_it;
    const subdomain_id_type & sid = elem->subdomain_id();

    // Always store centroid (even if not building kd-tree)
    _centroids_by_subdomain[sid].emplace_back(elem->vertex_average());

    _elem_id_map_by_subdomain[sid].emplace_back(elem->id());

    std::unique_ptr<SBMBndElementBase> bnd_elem;
    if (elem->type() == EDGE2)
      bnd_elem = std::make_unique<SBMBndEdge2>(elem);
    else if (elem->type() == TRI3)
      bnd_elem = std::make_unique<SBMBndTri3>(elem);
    else
      mooseError("Unsupported element type in SurfaceMeshBySubdomainBuilder");

    _boundary_elements_by_subdomain[sid].emplace_back(std::move(bnd_elem));
  }
}

bool
SurfaceMeshBySubdomainBuilder::checkWatertightness() const
{
  return SBMUtils::checkWatertightnessFromRawElems(std::vector<const Elem *>(
      _mesh->active_element_ptr_range().begin(), _mesh->active_element_ptr_range().end()));
}
