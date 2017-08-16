/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CircleCutUserObject.h"

// MOOSE includes
#include "MooseError.h"

// XFEM includes
#include "XFEMFuncs.h"

template <>
InputParameters
validParams<CircleCutUserObject>()
{
  // Get input parameters from parent class
  InputParameters params = validParams<GeometricCut3DUserObject>();

  // Add required parameters
  params.addRequiredParam<Point>("centroid", "Coordinates of the center point of the circle");
  params.addRequiredParam<Point>("edge_point1",
                                 "Coordinates of one point on the edge of the circle");
  params.addRequiredParam<Point>("edge_point2",
                                 "Coordinates of a second point on the edge of the circle");
  // Class description
  params.addClassDescription("Creates a UserObject for circular cuts on 3D meshes for XFEM");
  // Return the parameters
  return params;
}

CircleCutUserObject::CircleCutUserObject(const InputParameters & parameters)
  : GeometricCut3DUserObject(parameters),
    _centroid(getParam<Point>("centroid")),
    _edge_point1(getParam<Point>("edge_point1")),
    _edge_point2(getParam<Point>("edge_point2"))
{
  // Assign input data to vars used to construct cuts
  _center = _centroid;

  std::pair<Point, Point> rays = std::make_pair(_edge_point1 - _center, _edge_point2 - _center);

  _normal = rays.first.cross(rays.second);
  Xfem::normalizePoint(_normal);

  std::pair<Real, Real> ray_radii =
      std::make_pair(std::sqrt(rays.first.norm_sq()), std::sqrt(rays.second.norm_sq()));

  if (std::abs(ray_radii.first - ray_radii.second) > 1e-10)
    mooseError("CircleCutUserObject only works for a circular cut");

  _radius = 0.5 * (ray_radii.first + ray_radii.second);
  _angle = std::acos((rays.first * rays.second) / (ray_radii.first * ray_radii.second));
}

bool
CircleCutUserObject::isInsideCutPlane(Point p) const
{
  Point ray = p - _center;
  if (std::abs(ray * _normal) < 1e-15 && std::sqrt(ray.norm_sq()) < _radius)
    return true;
  return false;
}
