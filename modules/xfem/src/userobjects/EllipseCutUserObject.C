//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EllipseCutUserObject.h"

// MOOSE includes
#include "MooseError.h"

// XFEM includes
#include "XFEMFuncs.h"

registerMooseObject("XFEMApp", EllipseCutUserObject);

InputParameters
EllipseCutUserObject::validParams()
{
  // Get input parameters from parent class
  InputParameters params = GeometricCut3DUserObject::validParams();

  // Add required parameters
  params.addRequiredParam<std::vector<Real>>("cut_data",
                                             "Vector of Real values providing cut information");
  // Class description
  params.addClassDescription("Creates a UserObject for elliptical cuts on 3D meshes for XFEM");
  // Return the parameters
  return params;
}

EllipseCutUserObject::EllipseCutUserObject(const InputParameters & parameters)
  : GeometricCut3DUserObject(parameters), _cut_data(getParam<std::vector<Real>>("cut_data"))
{
  // Set up constant parameters
  const int cut_data_len = 9;

  // Throw error if length of cut_data is incorrect
  if (_cut_data.size() != cut_data_len)
    mooseError("Length of EllipseCutUserObject cut_data must be 9");

  // Assign cut_data to vars used to construct cuts
  _center = Point(_cut_data[0], _cut_data[1], _cut_data[2]);
  _vertices.push_back(Point(_cut_data[3], _cut_data[4], _cut_data[5]));
  _vertices.push_back(Point(_cut_data[6], _cut_data[7], _cut_data[8]));

  std::pair<Point, Point> rays = std::make_pair(_vertices[0] - _center, _vertices[1] - _center);

  if (std::abs(rays.first * rays.second) > 1e-6)
    mooseError(
        "EllipseCutUserObject only works on an elliptic cut. Users should provide two points at "
        "the long and short axis.");

  _normal = rays.first.cross(rays.second);
  Xfem::normalizePoint(_normal);

  std::pair<Real, Real> ray_radii =
      std::make_pair(std::sqrt(rays.first.norm_sq()), std::sqrt(rays.second.norm_sq()));

  // Determine which the long and short axes
  if (ray_radii.first > ray_radii.second)
  {
    _unit_vec1 = rays.first;
    _unit_vec2 = rays.second;
    _long_axis = ray_radii.first;
    _short_axis = ray_radii.second;
  }
  else
  {
    _unit_vec1 = rays.second;
    _unit_vec2 = rays.first;
    _long_axis = ray_radii.second;
    _short_axis = ray_radii.first;
  }

  Xfem::normalizePoint(_unit_vec1);
  Xfem::normalizePoint(_unit_vec2);
}

bool
EllipseCutUserObject::isInsideCutPlane(Point p) const
{
  Point ray = p - _center;
  if (std::abs(ray * _normal) < 1e-6)
  {
    std::pair<Real, Real> xy_loc = std::make_pair(ray * _unit_vec1, ray * _unit_vec2);

    if (std::sqrt(xy_loc.first * xy_loc.first / (_long_axis * _long_axis) +
                  xy_loc.second * xy_loc.second / (_short_axis * _short_axis)) < 1)
      return true;
  }
  return false;
}

const std::vector<Point>
EllipseCutUserObject::getCrackFrontPoints(unsigned int number_crack_front_points) const
{
  std::vector<Point> crack_front_points(number_crack_front_points);
  for (unsigned int i = 0; i < number_crack_front_points; ++i)
  {
    Real theta = 2.0 * libMesh::pi / number_crack_front_points * i;
    crack_front_points[i] = _center + _long_axis * std::sin(theta) * _unit_vec1 +
                            _short_axis * std::cos(theta) * _unit_vec2;
  }
  return crack_front_points;
}

const std::vector<RealVectorValue>
EllipseCutUserObject::getCrackPlaneNormals(unsigned int /*num_crack_front_points*/) const
{
  mooseError("getCrackPlaneNormals() is not implemented for this object.");
}
