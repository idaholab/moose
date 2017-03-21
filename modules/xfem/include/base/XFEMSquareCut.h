/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef XFEM_SQUARE_CUT_H
#define XFEM_SQUARE_CUT_H

#include "XFEMGeometricCut3D.h"

class XFEMSquareCut : public XFEMGeometricCut3D
{
public:
  XFEMSquareCut(std::vector<Real> square_nodes);
  ~XFEMSquareCut();

private:
  std::vector<Point> _vertices;

  bool isInsideCutPlane(Point p);
};

#endif
