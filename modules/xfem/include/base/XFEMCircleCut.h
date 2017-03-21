/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef XFEM_CIRCLE_CUT_H
#define XFEM_CIRCLE_CUT_H

#include "XFEMGeometricCut3D.h"

class XFEMCircleCut : public XFEMGeometricCut3D
{
public:
  XFEMCircleCut(std::vector<Real> square_nodes);
  ~XFEMCircleCut();

private:
  std::vector<Point> _vertices;
  Real _radius;
  Real _angle;

  virtual bool isInsideCutPlane(Point p);
};

#endif
