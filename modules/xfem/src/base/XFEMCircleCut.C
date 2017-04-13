/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "XFEMCircleCut.h"

#include "MooseError.h"
#include "EFAFuncs.h"
#include "XFEMFuncs.h"

XFEMCircleCut::XFEMCircleCut(std::vector<Real> circle_nodes)
  : XFEMGeometricCut3D(0.0, 0.0), _vertices(2, Point(0.0, 0.0, 0.0)), _radius(0.0), _angle(0.0)
{
  _center = Point(circle_nodes[0], circle_nodes[1], circle_nodes[2]);
  _vertices[0] = Point(circle_nodes[3], circle_nodes[4], circle_nodes[5]);
  _vertices[1] = Point(circle_nodes[6], circle_nodes[7], circle_nodes[8]);

  Point ray1 = _vertices[0] - _center;
  Point ray2 = _vertices[1] - _center;

  _normal = ray1.cross(ray2);
  Xfem::normalizePoint(_normal);

  Real R1 = std::sqrt(ray1.norm_sq());
  Real R2 = std::sqrt(ray2.norm_sq());
  if (std::abs(R1 - R2) > 1e-10)
    mooseError("XFEMCircleCut only works for a circular cut");

  _radius = 0.5 * (R1 + R2);
  _angle = std::acos((ray1 * ray2) / (R1 * R2));
}

XFEMCircleCut::~XFEMCircleCut() {}

bool
XFEMCircleCut::isInsideCutPlane(Point p)
{
  Point ray = p - _center;
  if (std::abs(ray * _normal) < 1e-15 && std::sqrt(ray.norm_sq()) < _radius)
    return true;
  return false;
}
