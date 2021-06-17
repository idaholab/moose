//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"

/**
 * Utility to produce the value and derivative of a cubic function at a point.  The cubic is defined
 * by values and derivatives at two points.
 */

namespace PorousFlowCubic
{
/**
 * Cubic function f(x) that satisfies
 * f(x0) = y0
 * f'(x0) = y0p
 * f(x1) = y1
 * f'(x1) = y1p
 * @param x the argument
 * @return value of the cubic function at x
 */
Real cubic(Real x, Real x0, Real y0, Real y0p, Real x1, Real y1, Real y1p);

/**
 * Derivative of cubic function, f(x), with respect to x.  f(x) satisfies
 * f(x0) = y0
 * f'(x0) = y0p
 * f(x1) = y1
 * f'(x1) = y1p
 * @param x the argument
 * @return the derivative of the cubic function at x
 */
Real dcubic(Real x, Real x0, Real y0, Real y0p, Real x1, Real y1, Real y1p);
}
