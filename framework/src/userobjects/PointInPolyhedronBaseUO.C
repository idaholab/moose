//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PointInPolyhedronBaseUO.h"

namespace
{
/// Map the input-file enum string to the typed backend selector.
PointContainmentClassifier::Method
methodFromEnum(const MooseEnum & method)
{
  if (method == "pca_ray")
    return PointContainmentClassifier::Method::PCA_RAY;
  if (method == "user_selected_ray")
    return PointContainmentClassifier::Method::USER_SELECTED_RAY;
  return PointContainmentClassifier::Method::FIXED_X_RAY;
}
}

InputParameters
PointInPolyhedronBaseUO::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription("Base class for user objects that determine whether a point is inside "
                             "a closed surface mesh.");

  MooseEnum methods("pca_ray user_selected_ray fixed_x_ray", "pca_ray");
  params.addParam<MooseEnum>(
      "point_containment_method",
      methods,
      "Algorithm used for point-containment queries. 'pca_ray' (default) uses the ray-casting "
      "engine with a PCA-selected ray; 'user_selected_ray' uses the ray-casting engine with the "
      "'ray_direction' parameter; 'fixed_x_ray' uses the TriangleManifold engine (fixed +x ray, "
      "TRI3 surfaces only).");

  params.addParam<Point>(
      "ray_direction",
      Point(0.0, 0.0, 0.0),
      "Ray direction for the 'user_selected_ray' method, used exactly as given (no "
      "auto-selection). Any finite non-zero direction is allowed, including oblique; for a 2D "
      "surface it must lie in the mesh plane (zero z component). The user is responsible for "
      "avoiding directions that graze vertices/edges or are tangent to the surface. Other "
      "methods ignore this parameter.");

  params.addParam<Real>("tolerance",
                        libMesh::TOLERANCE,
                        "Tolerance used for intersection or surface-proximity checks. Determines "
                        "whether a point is considered on the surface.");

  params.addParam<int>(
      "leaf_max_size", 10, "Maximum number of elements in a leaf node of the KD-tree.");

  params.addParam<FileName>("obb_file_name", "", "Oriented Bounding Box (OBB) debug file name.");
  params.addParam<FileName>("ray_file_name", "", "Ray debug file name.");

  return params;
}

PointInPolyhedronBaseUO::PointInPolyhedronBaseUO(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _method(methodFromEnum(getParam<MooseEnum>("point_containment_method"))),
    _ray_direction(getParam<Point>("ray_direction")),
    _tolerance(getParam<Real>("tolerance")),
    _leaf_max_size(getParam<int>("leaf_max_size")),
    _obb_file_name(getParam<FileName>("obb_file_name")),
    _ray_file_name(getParam<FileName>("ray_file_name"))
{
  if (_leaf_max_size <= 0)
    paramError("leaf_max_size", "must be greater than zero.");
  if (_tolerance <= 0.0)
    paramError("tolerance", "must be greater than zero.");

  const bool ray_direction_set = !_ray_direction.absolute_fuzzy_equals(Point(0.0, 0.0, 0.0));

  // user_selected_ray needs a direction; the other methods ignore it.
  if (_method == PointContainmentClassifier::Method::USER_SELECTED_RAY && !ray_direction_set)
    paramError(
        "ray_direction",
        "must be set to a non-zero vector when point_containment_method = user_selected_ray.");
  if (_method != PointContainmentClassifier::Method::USER_SELECTED_RAY && ray_direction_set)
    paramError("ray_direction", "is only used by point_containment_method = user_selected_ray.");

  // The fixed_x_ray (TriangleManifold) backend does not emit OBB/ray debug files.
  if (_method == PointContainmentClassifier::Method::FIXED_X_RAY &&
      (!_obb_file_name.empty() || !_ray_file_name.empty()))
    mooseInfo("point_containment_method = fixed_x_ray does not produce OBB/ray debug files; "
              "obb_file_name/ray_file_name will be ignored.");
}

PointContainmentClassifier::RayOptions
PointInPolyhedronBaseUO::pcaRayOptions() const
{
  PointContainmentClassifier::RayOptions options;
  options.ray_direction = _ray_direction;
  options.leaf_max_size = _leaf_max_size;
  options.obb_file_name = _obb_file_name;
  options.ray_file_name = _ray_file_name;
  options.comm = &comm();
  return options;
}
