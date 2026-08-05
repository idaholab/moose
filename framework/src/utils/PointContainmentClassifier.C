//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PointContainmentClassifier.h"
#include "SurfaceElementSet.h"
#include "AdaptiveRayContainmentCheck.h"
#include "TriangleManifold.h"
#include "MooseError.h"

PointContainmentClassifier::PointContainmentClassifier(MeshBase & mesh,
                                                       const SurfaceElementSet * set,
                                                       PointContainmentMethod method,
                                                       Real tolerance,
                                                       const PcaRayOptions & pca)
  : _method(method)
{
  switch (_method)
  {
    case PointContainmentMethod::PCA_RAY:
    case PointContainmentMethod::USER_SELECTED_RAY:
    {
      if (!set)
        mooseError(
            "PointContainmentClassifier: a SurfaceElementSet is required for the pca_ray and "
            "user_selected_ray methods.");

      // pca_ray uses AUTO_PCA; user_selected_ray passes the user's direction as USER_SPECIFIED.
      // Both are carried in pca.ray_direction (a RayDirectionOptions).
      _pca = std::make_unique<AdaptiveRayContainmentCheck>(set->elements(),
                                                           set->centroids(),
                                                           pca.ray_direction,
                                                           tolerance,
                                                           pca.leaf_max_size,
                                                           pca.obb_file_name,
                                                           pca.ray_file_name,
                                                           pca.comm);

      _bounding_box = set->boundingBox();
      _num_elements = set->size();
      break;
    }

    case PointContainmentMethod::FIXED_X_RAY:
    {
      _tri = std::make_unique<TriangleManifold>(mesh, tolerance);
      _bounding_box = _tri->boundingBox();
      _num_elements = _tri->numTriangles();
      break;
    }
  }
}

PointContainmentClassifier::~PointContainmentClassifier() = default;

SurfaceSide
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
