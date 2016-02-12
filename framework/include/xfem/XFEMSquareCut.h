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
