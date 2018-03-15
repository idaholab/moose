//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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

inline Real
heavyside(Real x)
{
  return x < 0.0 ? 0.0 : 1.0;
}
inline Real
positivePart(Real x)
{
  return x > 0.0 ? x : 0.0;
}
inline Real
negativePart(Real x)
{
  return x < 0.0 ? x : 0.0;
}

} // namespace MathUtils

#endif // MATHUTILS_H
