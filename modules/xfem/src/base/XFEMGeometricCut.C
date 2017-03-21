/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "XFEMGeometricCut.h"

XFEMGeometricCut::XFEMGeometricCut(Real t0, Real t1) : _t_start(t0), _t_end(t1) {}

XFEMGeometricCut::~XFEMGeometricCut() {}

Real
XFEMGeometricCut::cutFraction(Real time)
{
  Real fraction = 0.0;
  if (time >= _t_start)
  {
    if (time >= _t_end)
      fraction = 1.0;
    else
      fraction = (time - _t_start) / (_t_end - _t_start);
  }
  return fraction;
}

Real
XFEMGeometricCut::crossProduct2D(Real ax, Real ay, Real bx, Real by)
{
  return (ax * by - bx * ay);
}
