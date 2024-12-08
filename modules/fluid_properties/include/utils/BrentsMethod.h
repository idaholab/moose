//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Moose.h"
#include "MooseTypes.h"

// C++ includes
#include <functional>

namespace BrentsMethod
{
/**
 * Function to bracket a root of a given function. Adapted from
 * Numerical Recipes in C
 *
 * @param f reference to function to find bracketing interval
 * @param[out] x1 reference one bound
 * @param[out] x2 reference to other bound
 */
void bracket(std::function<Real(Real)> const & f, Real & x1, Real & x2);

/**
 * Finds the root of a function using Brent's method. Adapted from
 * Numerical Recipes in C
 *
 * @param f reference to function to find root of
 * @param x1 one end of bracketing interval
 * @param x2 other end of bracketing interval
 * @param tolerance root finding tolerance (default is 1e-12)
 */
Real root(std::function<Real(Real)> const & f, Real x1, Real x2, Real tol = 1.0e-12);
}
