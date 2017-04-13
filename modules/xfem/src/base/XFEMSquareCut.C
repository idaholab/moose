/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "XFEMSquareCut.h"

#include "MooseError.h"
#include "XFEMFuncs.h"
#include "EFAFuncs.h"

XFEMSquareCut::XFEMSquareCut(std::vector<Real> square_nodes)
  : XFEMGeometricCut3D(0.0, 0.0), _vertices(4, Point(0.0, 0.0, 0.0))
{
  _vertices[0] = Point(square_nodes[0], square_nodes[1], square_nodes[2]);
  _vertices[1] = Point(square_nodes[3], square_nodes[4], square_nodes[5]);
  _vertices[2] = Point(square_nodes[6], square_nodes[7], square_nodes[8]);
  _vertices[3] = Point(square_nodes[9], square_nodes[10], square_nodes[11]);

  for (unsigned int i = 0; i < 4; ++i)
    _center += _vertices[i];
  _center *= 0.25;

  for (unsigned int i = 0; i < 4; ++i)
  {
    unsigned int iplus1(i < 3 ? i + 1 : 0);
    Point ray1 = _vertices[i] - _center;
    Point ray2 = _vertices[iplus1] - _center;
    _normal += ray1.cross(ray2);
  }
  _normal *= 0.25;
  Xfem::normalizePoint(_normal);
}

XFEMSquareCut::~XFEMSquareCut() {}

bool
XFEMSquareCut::isInsideCutPlane(Point p)
{
  bool inside = false;
  unsigned int counter = 0;
  for (unsigned int i = 0; i < 4; ++i)
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
  if (counter == 4)
    inside = true;
  return inside;
}
