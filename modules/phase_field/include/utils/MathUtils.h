/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef MATHUTILS_H
#define MATHUTILS_H

#include "Moose.h"
#include "libmesh/libmesh.h"
#include "libmesh/utility.h"

namespace MathUtils
{

inline Real
round(Real x)
{
  return ::round(x); // use round from math.h
}

inline Real
sign(Real x)
{
  return x >= 0.0 ? 1.0 : -1.0;
}

Real poly1Log(Real x, Real tol, int deriv);
Real poly2Log(Real x, Real tol, int deriv);
Real poly3Log(Real x, Real tol, int order);
Real poly4Log(Real x, Real tol, int order);
Real taylorLog(Real x);

Real pow(Real x, unsigned int e);

} // namespace MathUtils

#endif // MATHUTILS_H
