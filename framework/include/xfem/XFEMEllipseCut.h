/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
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
