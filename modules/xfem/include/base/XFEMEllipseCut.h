/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef XFEM_ELLIPSE_CUT_H
#define XFEM_ELLIPSE_CUT_H

#include "XFEMGeometricCut3D.h"

class XFEMEllipseCut : public XFEMGeometricCut3D
{
public:
  XFEMEllipseCut(std::vector<Real> square_nodes);
  ~XFEMEllipseCut();

private:
  std::vector<Point> _vertices;
  Point _unit_vec1;
  Point _unit_vec2;
  Real _long_axis;
  Real _short_axis;

  virtual bool isInsideCutPlane(Point p);
};

#endif
