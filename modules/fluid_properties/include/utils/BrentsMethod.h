/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef BRENTSMETHOD_H
#define BRENTSMETHOD_H

#include "Moose.h"

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

#endif // BRENTSMETHOD_H
