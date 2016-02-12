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

#include "XFEMGeometricCut.h"

XFEMGeometricCut::XFEMGeometricCut(Real t0, Real t1):
  t_start(t0),
  t_end(t1)
{}

XFEMGeometricCut::~XFEMGeometricCut()
{}

Real XFEMGeometricCut::cutFraction(Real time)
{
  Real fraction = 0.0;
  if (time > t_start)
  {
    if (time >= t_end)
    {
      fraction = 1.0;
    }
    else
    {
      fraction = (time - t_start) / (t_end - t_start);
    }
  }
  return fraction;
}

Real XFEMGeometricCut::crossProduct2D(Real ax, Real ay, Real bx, Real by)
{
  return (ax*by-bx*ay);
}
