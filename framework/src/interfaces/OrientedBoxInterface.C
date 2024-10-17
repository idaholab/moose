//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OrientedBoxInterface.h"

// MOOSE includes
#include "InputParameters.h"
#include "MooseError.h"

#include <memory>

InputParameters
OrientedBoxInterface::validParams()
{

  InputParameters params = emptyInputParameters();
  params.addRequiredParam<Point>("center",
                                 "The center (many people spell this 'center') of the box.");
  params.addRequiredParam<Real>("width", "The width of the box");
  params.addRequiredParam<Real>("length", "The length of the box");
  params.addRequiredParam<Real>("height", "The height of the box");
  params.addRequiredParam<RealVectorValue>("width_direction",
                                           "The direction along which the width is oriented.");
  params.addRequiredParam<RealVectorValue>("length_direction",
                                           "The direction along which the length is oriented (must "
                                           "be perpendicular to width_direction).");
  return params;
}

OrientedBoxInterface::OrientedBoxInterface(const InputParameters & parameters)
  : _center(parameters.get<Point>("center"))
{
  const std::string & name = parameters.get<std::string>("_object_name");

  // Define the bounding box
  Real xmax = 0.5 * parameters.get<Real>("width");
  Real ymax = 0.5 * parameters.get<Real>("length");
  Real zmax = 0.5 * parameters.get<Real>("height");

  Point bottom_left(-xmax, -ymax, -zmax);
  Point top_right(xmax, ymax, zmax);

  _bounding_box = std::make_unique<libMesh::BoundingBox>(bottom_left, top_right);

  /*
   * now create the rotation matrix that rotates the oriented
   * box's width direction to "x", its length direction to "y"
   * and its height direction to "z"
   */
  RealVectorValue w = parameters.get<RealVectorValue>("width_direction");
  RealVectorValue l = parameters.get<RealVectorValue>("length_direction");

  /*
   * Normalize the width and length directions in readiness for
   * insertion into the rotation matrix
   */
  Real len = w.norm();
  if (len == 0.0)
    mooseError("Length of width_direction vector is zero in ", name);
  w /= len;

  len = l.norm();
  if (len == 0.0)
    mooseError("Length of length_direction vector is zero in ", name);
  l /= len;

  if (w * l > 1E-10)
    mooseError("width_direction and length_direction are not perpendicular in ", name);

  // The rotation matrix!
  _rot_matrix = std::make_unique<RealTensorValue>(w, l, w.cross(l));
}

bool
OrientedBoxInterface::containsPoint(const Point & point)
{
  // Translate the point to the origin, and then rotate
  return _bounding_box->contains_point((*_rot_matrix) * (point - _center));
}
