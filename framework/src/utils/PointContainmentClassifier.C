//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PointContainmentClassifier.h"
#include "RayDirectionOptions.h"
#include "SurfaceElementSet.h"
#include "AdaptiveRayContainmentCheck.h"
#include "TriangleManifold.h"
#include "MooseError.h"

namespace
{
PointContainmentClassifier::RayOptions
defaultRayOptions(const PointContainmentClassifier::Method method)
{
  if (method == PointContainmentClassifier::Method::USER_SELECTED_RAY)
    mooseError("PointContainmentClassifier: user_selected_ray requires explicit RayOptions.");

  return {};
}
} // namespace

PointContainmentClassifier::PointContainmentClassifier(MeshBase & mesh,
                                                       const SurfaceElementSet * set,
                                                       Method method,
                                                       Real tolerance)
  : PointContainmentClassifier(mesh, set, method, tolerance, defaultRayOptions(method))
{
}

PointContainmentClassifier::PointContainmentClassifier(
    MeshBase & mesh,
    const SurfaceElementSet * set,
    PointContainmentClassifier::Method method,
    Real tolerance,
    const PointContainmentClassifier::RayOptions & options)
  : _method(method)
{
  switch (_method)
  {
    case PointContainmentClassifier::Method::PCA_RAY:
    case PointContainmentClassifier::Method::USER_SELECTED_RAY:
    {
      if (!set)
        mooseError(
            "PointContainmentClassifier: a SurfaceElementSet is required for the pca_ray and "
            "user_selected_ray methods.");

      const SurfaceGeometry::RayDirectionOptions ray_options{
          _method == Method::PCA_RAY ? SurfaceGeometry::RayDirectionMode::AUTO_PCA
                                     : SurfaceGeometry::RayDirectionMode::USER_SPECIFIED,
          options.ray_direction};
      _pca = std::make_unique<AdaptiveRayContainmentCheck>(set->elements(),
                                                           set->centroids(),
                                                           ray_options,
                                                           tolerance,
                                                           options.leaf_max_size,
                                                           options.obb_file_name,
                                                           options.ray_file_name,
                                                           options.comm);

      _bounding_box = set->boundingBox();
      _num_elements = set->size();
      break;
    }

    case PointContainmentClassifier::Method::FIXED_X_RAY:
    {
      _tri = std::make_unique<TriangleManifold>(mesh, tolerance);
      _bounding_box = _tri->boundingBox();
      _num_elements = _tri->numTriangles();
      break;
    }
  }
}

PointContainmentClassifier::~PointContainmentClassifier() = default;

SurfaceGeometry::SurfaceSide
PointContainmentClassifier::sideness(const Point & point) const
{
  if (_tri)
    return _tri->sideness(point);

  mooseAssert(_pca, "PointContainmentClassifier: no backend was constructed.");
  return _pca->sideness(point);
}

Point
PointContainmentClassifier::rayDirection() const
{
  if (!_pca)
    mooseError("PointContainmentClassifier::rayDirection() is only defined for the pca_ray and "
               "user_selected_ray methods; the fixed_x_ray method has no ray-casting backend.");
  return _pca->rayDirection();
}
