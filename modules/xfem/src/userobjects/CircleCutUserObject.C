//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CircleCutUserObject.h"

// MOOSE includes
#include "MooseError.h"

// XFEM includes
#include "XFEMFuncs.h"

registerMooseObject("XFEMApp", CircleCutUserObject);

InputParameters
CircleCutUserObject::validParams()
{
  // Get input parameters from parent class
  InputParameters params = GeometricCut3DUserObject::validParams();

  // Add required parameters
  params.addRequiredParam<std::vector<Real>>("cut_data",
                                             "Vector of Real values providing cut information");
  // Class description
  params.addClassDescription("Creates a UserObject for circular cuts on 3D meshes for XFEM");
  // Return the parameters
  return params;
}

CircleCutUserObject::CircleCutUserObject(const InputParameters & parameters)
  : GeometricCut3DUserObject(parameters), _cut_data(getParam<std::vector<Real>>("cut_data"))
{
  // Set up constant parameters
  const int cut_data_len = 9;

  // Throw error if length of cut_data is incorrect
  if (_cut_data.size() != cut_data_len)
    mooseError("Length of CircleCutUserObject cut_data must be 9");

  // Assign cut_data to vars used to construct cuts
  _center = Point(_cut_data[0], _cut_data[1], _cut_data[2]);
  _vertices.push_back(Point(_cut_data[3], _cut_data[4], _cut_data[5]));
  _vertices.push_back(Point(_cut_data[6], _cut_data[7], _cut_data[8]));

  std::pair<Point, Point> rays = std::make_pair(_vertices[0] - _center, _vertices[1] - _center);

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

const std::vector<Point>
CircleCutUserObject::getCrackFrontPoints(unsigned int number_crack_front_points) const
{
  std::vector<Point> crack_front_points(number_crack_front_points);
  Point v1 = _vertices[0] - _center;
  Point v2 = _normal.cross(v1);
  v1 /= v1.norm();
  v2 /= v2.norm();
  // parametric circle in 3D: center + r * cos(theta) * v1 + r * sin(theta) * v2
  for (unsigned int i = 0; i < number_crack_front_points; ++i)
  {
    Real theta = 2.0 * libMesh::pi / number_crack_front_points * i;
    crack_front_points[i] =
        _center + _radius * std::cos(theta) * v1 + _radius * std::sin(theta) * v2;
  }

  return crack_front_points;
}

const std::vector<RealVectorValue>
CircleCutUserObject::getCrackPlaneNormals(unsigned int /*num_crack_front_points*/) const
{
  mooseError("getCrackPlaneNormals() is not implemented for this object.");
}
