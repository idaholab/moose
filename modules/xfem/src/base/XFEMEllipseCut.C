/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "XFEMEllipseCut.h"

#include "EFAFuncs.h"
#include "XFEMFuncs.h"
#include "MooseError.h"

XFEMEllipseCut::XFEMEllipseCut(std::vector<Real> ellipse_nodes)
  : XFEMGeometricCut3D(0.0, 0.0),
    _vertices(2, Point(0.0, 0.0, 0.0)),
    _unit_vec1(Point(0.0, 0.0, 0.0)),
    _unit_vec2(Point(0.0, 0.0, 0.0)),
    _long_axis(0.0),
    _short_axis(0.0)
{
  _center = Point(ellipse_nodes[0], ellipse_nodes[1], ellipse_nodes[2]);
  _vertices[0] = Point(ellipse_nodes[3], ellipse_nodes[4], ellipse_nodes[5]);
  _vertices[1] = Point(ellipse_nodes[6], ellipse_nodes[7], ellipse_nodes[8]);

  Point ray1 = _vertices[0] - _center;
  Point ray2 = _vertices[1] - _center;

  if (std::abs(ray1 * ray2) > 1e-6)
    mooseError("XFEMEllipseCut only works on an elliptic cut. Users should provide two points at "
               "the long and short axis.");

  _normal = ray1.cross(ray2);
  Xfem::normalizePoint(_normal);

  Real R1 = std::sqrt(ray1.norm_sq());
  Real R2 = std::sqrt(ray2.norm_sq());

  if (R1 > R2)
  {
    _unit_vec1 = ray1;
    _unit_vec2 = ray2;
    _long_axis = R1;
    _short_axis = R2;
  }
  else
  {
    _unit_vec1 = ray2;
    _unit_vec2 = ray1;
    _long_axis = R2;
    _short_axis = R1;
  }

  Xfem::normalizePoint(_unit_vec1);
  Xfem::normalizePoint(_unit_vec2);
}

XFEMEllipseCut::~XFEMEllipseCut() {}

bool
XFEMEllipseCut::isInsideCutPlane(Point p)
{
  Point ray = p - _center;
  if (std::abs(ray * _normal) < 1e-6)
  {
    double xloc = ray * _unit_vec1;
    double yloc = ray * _unit_vec2;

    if (std::sqrt(xloc * xloc / (_long_axis * _long_axis) +
                  yloc * yloc / (_short_axis * _short_axis)) < 1)
      return true;
  }
  return false;
}
