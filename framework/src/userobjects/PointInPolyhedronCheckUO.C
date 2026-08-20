//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PointInPolyhedronCheckUO.h"
#include "BoundaryMeshBuilder.h"

#include "libmesh/elem.h"
#include "libmesh/string_to_enum.h"

registerMooseObject("MooseApp", PointInPolyhedronCheckUO);

InputParameters
PointInPolyhedronCheckUO::validParams()
{
  InputParameters params = PointInPolyhedronBaseUO::validParams();
  params.addClassDescription("Determines whether a point is inside a closed surface mesh provided "
                             "by a BoundaryMeshBuilder.");

  params.addRequiredParam<UserObjectName>(
      "builder", "The BoundaryMeshBuilder providing the surface mesh and boundary elements.");

  return params;
}

PointInPolyhedronCheckUO::PointInPolyhedronCheckUO(const InputParameters & parameters)
  : PointInPolyhedronBaseUO(parameters),
    _builder(getUserObject<BoundaryMeshBuilder>("builder")),
    _classifier(nullptr)
{
}

void
PointInPolyhedronCheckUO::initialSetup()
{
  // The fixed_x_ray backend (TriangleManifold) handles only 3D TRI3 surface
  // meshes (mesh_dimension 2). Reject 2D EDGE2 surfaces here with a clear
  // message; use pca_ray or user_selected_ray for 2D instead.
  if (_method == PointContainmentClassifier::Method::FIXED_X_RAY)
  {
    if (_builder.mesh().mesh_dimension() != 2)
      paramError("point_containment_method",
                 "fixed_x_ray (the TriangleManifold engine) supports only 3D TRI3 surface meshes. "
                 "For a 2D (EDGE2) surface use pca_ray, or user_selected_ray with "
                 "ray_direction = '1 0 0'.");

    // TriangleManifold accepts TRI3 only. Validate the element type up front so an
    // unsupported surface (e.g. QUAD4) fails with a method-aware message here rather
    // than inside the later manifold construction.
    for (const auto * elem : _builder.mesh().active_element_ptr_range())
      if (elem->type() != TRI3)
        paramError("point_containment_method",
                   "fixed_x_ray (the TriangleManifold engine) supports only TRI3 surface elements, "
                   "but the surface mesh contains ",
                   libMesh::Utility::enum_to_string(elem->type()),
                   ". Use pca_ray or user_selected_ray instead.");
  }

  // The fixed_x_ray (TriangleManifold) backend classifies from the mesh directly
  // and ignores the SurfaceElementSet. Skip surfaceElementSet() in that case so
  // the builder never builds the whole-mesh set that would go unused.
  const SurfaceElementSet * const set = (_method == PointContainmentClassifier::Method::FIXED_X_RAY)
                                            ? nullptr
                                            : &_builder.surfaceElementSet();
  _classifier = std::make_unique<PointContainmentClassifier>(
      _builder.mesh(), set, _method, _tolerance, pcaRayOptions());
}
