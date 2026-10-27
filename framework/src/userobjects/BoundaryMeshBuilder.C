//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BoundaryMeshBuilder.h"
#include "MooseMesh.h"
#include "MeshGeneratorSystem.h"

registerMooseObject("MooseApp", BoundaryMeshBuilder);

InputParameters
BoundaryMeshBuilder::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription(
      "Owns a saved surface (boundary) mesh and the SurfaceElement wrappers built from it, "
      "for use by point-containment and distance user objects.");

  params.addRequiredParam<std::string>(
      "surface_mesh",
      "The name of the surface mesh saved via the MeshGenerator's `save_with_name` parameter.");

  params.addParam<bool>(
      "check_watertightness",
      false,
      "Check if the mesh is watertight. If false, the mesh may not be suitable for In-Out tests.");

  return params;
}

BoundaryMeshBuilder::BoundaryMeshBuilder(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _bnd_mesh_name(getParam<std::string>("surface_mesh")),
    _check_watertightness(getParam<bool>("check_watertightness")),
    _dim_embedding_mesh(_fe_problem.mesh().dimension() /*MooseMesh*/)
{
}

void
BoundaryMeshBuilder::initialSetup()
{
  // A saved mesh has single-retrieval semantics: getSavedMesh() std::move()s it
  // out and errors on a second retrieval. This builder is the sole owner.
  _mesh = _app.getMeshGeneratorSystem().getSavedMesh(_bnd_mesh_name);

  // A saved mesh produced by a MeshGenerator may be unprepared, leaving
  // mesh_dimension() stale and element neighbor links unset. Prepare it so the
  // dimension check and the neighbor-based watertightness test are reliable.
  _mesh->prepare_for_use();

  if (!_mesh->is_replicated())
    mooseError("BoundaryMeshBuilder '",
               name(),
               "': the saved surface mesh is distributed. A serialized/replicated surface mesh is "
               "required for point-containment queries.");

  const auto expected_dim_embedding_mesh = _mesh->mesh_dimension() + 1;
  if (_dim_embedding_mesh != expected_dim_embedding_mesh)
    mooseError("BoundaryMeshBuilder '",
               name(),
               "': the background mesh dimension (",
               _dim_embedding_mesh,
               ") does not match the surface mesh dimension + 1 (",
               expected_dim_embedding_mesh,
               ").");

  if (_check_watertightness)
  {
    if (checkWatertightness())
      mooseInfo("The mesh is watertight. It is suitable for In-Out tests.");
    else
      mooseInfo("The mesh is not watertight. It may not be suitable for In-Out tests.");
  }
}

void
BoundaryMeshBuilder::buildDefaultSet() const
{
  _set = std::make_unique<SurfaceElementSet>(SurfaceElementSet::fromMesh(*_mesh));
}

bool
BoundaryMeshBuilder::checkWatertightness() const
{
  for (const auto * elem : _mesh->active_element_ptr_range())
    for (const auto s : make_range(elem->n_sides()))
      if (!elem->neighbor_ptr(s))
        return false;

  return true;
}

MeshBase &
BoundaryMeshBuilder::mesh() const
{
  mooseAssert(_mesh, "BoundaryMeshBuilder mesh is not available until initialSetup().");
  return *_mesh;
}

const SurfaceElementSet &
BoundaryMeshBuilder::surfaceElementSet() const
{
  // Build the whole-mesh set on first use so backends that never need it (e.g.
  // the fixed_x_ray TriangleManifold engine) pay no allocation for it.
  if (!_set)
    buildDefaultSet();

  if (!_set)
    mooseError("BoundaryMeshBuilder '", name(), "': buildDefaultSet() did not initialize _set.");
  return *_set;
}
