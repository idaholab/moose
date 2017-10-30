/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "RectangleCutUserObject.h"

// MOOSE includes
#include "MooseError.h"

// XFEM includes
#include "XFEMFuncs.h"

template <>
InputParameters
validParams<RectangleCutUserObject>()
{
  // Get input parameters from parent class
  InputParameters params = validParams<GeometricCut3DUserObject>();

  // Add required parameters
  params.addRequiredParam<Point>(
      "vertex_point1", "Coordinates of the first of four vertices defining the rectangular cut");
  params.addRequiredParam<Point>(
      "vertex_point2", "Coordinates of the second of four vertices defining the rectangular cut");
  params.addRequiredParam<Point>(
      "vertex_point3", "Coordinates of the third of four vertices defining the rectangular cut");
  params.addRequiredParam<Point>(
      "vertex_point4", "Coordinates of the last of four vertices defining the rectangular cut");
  // Class description
  params.addClassDescription("Creates a UserObject for planar cuts on 3D meshes for XFEM");
  // Return the parameters
  return params;
}

RectangleCutUserObject::RectangleCutUserObject(const InputParameters & parameters)
  : GeometricCut3DUserObject(parameters),
    _vertex_point1(getParam<Point>("vertex_point1")),
    _vertex_point2(getParam<Point>("vertex_point2")),
    _vertex_point3(getParam<Point>("vertex_point3")),
    _vertex_point4(getParam<Point>("vertex_point4"))
{
  // Assign cut_data to vars used to construct cuts
  _vertices.push_back(_vertex_point1);
  _vertices.push_back(_vertex_point2);
  _vertices.push_back(_vertex_point3);
  _vertices.push_back(_vertex_point4);

  for (unsigned int i = 0; i < _vertices.size(); ++i)
    _center += _vertices[i];
  _center *= 0.25;

  for (unsigned int i = 0; i < _vertices.size(); ++i)
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
  bool inside = false;
  unsigned int counter = 0;
  for (unsigned int i = 0; i < _vertices.size(); ++i)
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
  if (counter == _vertices.size())
    inside = true;
  return inside;
}
