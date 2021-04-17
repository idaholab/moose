//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RectangleCutUserObject.h"

// MOOSE includes
#include "MooseError.h"

// XFEM includes
#include "XFEMFuncs.h"

registerMooseObject("XFEMApp", RectangleCutUserObject);

InputParameters
RectangleCutUserObject::validParams()
{
  // Get input parameters from parent class
  InputParameters params = GeometricCut3DUserObject::validParams();

  // Add required parameters
  params.addRequiredParam<std::vector<Real>>("cut_data",
                                             "Vector of Real values providing cut information");
  // Class description
  params.addClassDescription("Creates a UserObject for planar cuts on 3D meshes for XFEM");
  // Return the parameters
  return params;
}

RectangleCutUserObject::RectangleCutUserObject(const InputParameters & parameters)
  : GeometricCut3DUserObject(parameters), _cut_data(getParam<std::vector<Real>>("cut_data"))
{
  // Set up constant parameters
  const int cut_data_len = 12;
  const int num_vertices = 4;

  // Throw error if length of cut_data is incorrect
  if (_cut_data.size() != cut_data_len)
    mooseError("Length of RectangleCutUserObject cut_data must be 12");

  // Assign cut_data to vars used to construct cuts
  _vertices.push_back(Point(_cut_data[0], _cut_data[1], _cut_data[2]));
  _vertices.push_back(Point(_cut_data[3], _cut_data[4], _cut_data[5]));
  _vertices.push_back(Point(_cut_data[6], _cut_data[7], _cut_data[8]));
  _vertices.push_back(Point(_cut_data[9], _cut_data[10], _cut_data[11]));

  for (unsigned int i = 0; i < num_vertices; ++i)
    _center += _vertices[i];
  _center *= 0.25;

  for (unsigned int i = 0; i < num_vertices; ++i)
  {
    unsigned int iplus1(i < 3 ? i + 1 : 0);
    std::pair<Point, Point> rays =
        std::make_pair(_vertices[i] - _center, _vertices[iplus1] - _center);
    _normal += rays.first.cross(rays.second);
  }
  _normal *= 0.25;
  Xfem::normalizePoint(_normal);
}

bool
RectangleCutUserObject::isInsideCutPlane(Point p) const
{
  const int num_vertices = 4;

  bool inside = false;
  unsigned int counter = 0;
  for (unsigned int i = 0; i < num_vertices; ++i)
  {
    unsigned int iplus1 = (i < 3 ? i + 1 : 0);
    Point middle2p = p - 0.5 * (_vertices[i] + _vertices[iplus1]);
    const Point side_tang = _vertices[iplus1] - _vertices[i];
    Point side_norm = side_tang.cross(_normal);
    Xfem::normalizePoint(middle2p);
    Xfem::normalizePoint(side_norm);
    if (middle2p * side_norm <= 0.0)
      counter += 1;
  }
  if (counter == num_vertices)
    inside = true;
  return inside;
}

const std::vector<Point>
RectangleCutUserObject::getCrackFrontPoints(unsigned int /*num_crack_front_points*/) const
{
  mooseError("getCrackFrontPoints() is not implemented for this object.");
}

const std::vector<RealVectorValue>
RectangleCutUserObject::getCrackPlaneNormals(unsigned int /*num_crack_front_points*/) const
{
  mooseError("getCrackPlaneNormals() is not implemented for this object.");
}
