/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "EllipseCutUserObject.h"

// MOOSE includes
#include "MooseError.h"

// XFEM includes
#include "XFEMFuncs.h"

template <>
InputParameters
validParams<EllipseCutUserObject>()
{
  // Get input parameters from parent class
  InputParameters params = validParams<GeometricCut3DUserObject>();

  // Add required parameters
  params.addRequiredParam<Point>("centroid", "Coordinates of the center point of the ellipse");
  params.addRequiredParam<Point>("long_axis_point",
                                 "Coordinates of a long axis point on the edge of the ellipse");
  params.addRequiredParam<Point>("short_axis_point",
                                 "Coortinates of a short axis point on the edge of the ellipse");
  // Class description
  params.addClassDescription("Creates a UserObject for elliptical cuts on 3D meshes for XFEM");
  // Return the parameters
  return params;
}

EllipseCutUserObject::EllipseCutUserObject(const InputParameters & parameters)
  : GeometricCut3DUserObject(parameters),
    _centroid(getParam<Point>("centroid")),
    _long_axis_point(getParam<Point>("long_axis_point")),
    _short_axis_point(getParam<Point>("short_axis_point"))
{
  // Assign input data to vars used to construct cuts
  _center = _centroid;

  std::pair<Point, Point> rays =
      std::make_pair(_long_axis_point - _center, _short_axis_point - _center);

  if (std::abs(rays.first * rays.second) > 1e-6)
    mooseError(
        "EllipseCutUserObject only works on an elliptic cut. Users should provide two points at "
        "the long and short axis.");

  _normal = rays.first.cross(rays.second);
  Xfem::normalizePoint(_normal);

  std::pair<Real, Real> ray_radii =
      std::make_pair(std::sqrt(rays.first.norm_sq()), std::sqrt(rays.second.norm_sq()));

  // Determine which are the long and short axes
  if (ray_radii.first > ray_radii.second)
  {
    _long_axis_unit_vector = rays.first;
    _short_axis_unit_vector = rays.second;
    _long_axis = ray_radii.first;
    _short_axis = ray_radii.second;
  }
  else
  {
    mooseError("long_axis_point is closer to centroid point than short_axis_point in "
               "EllipseCutUserObject!");
  }

  Xfem::normalizePoint(_long_axis_unit_vector);
  Xfem::normalizePoint(_short_axis_unit_vector);
}

bool
EllipseCutUserObject::isInsideCutPlane(Point p) const
{
  Point ray = p - _center;
  if (std::abs(ray * _normal) < 1e-6)
  {
    std::pair<Real, Real> xy_loc =
        std::make_pair(ray * _long_axis_unit_vector, ray * _short_axis_unit_vector);

    if (std::sqrt(xy_loc.first * xy_loc.first / (_long_axis * _long_axis) +
                  xy_loc.second * xy_loc.second / (_short_axis * _short_axis)) < 1)
      return true;
  }
  return false;
}
