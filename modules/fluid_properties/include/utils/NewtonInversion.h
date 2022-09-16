// * This file is part of the MOOSE framework
// * https://www.mooseframework.org
// *
// * All rights reserved, see COPYRIGHT for full restrictions
// * https://github.com/idaholab/moose/blob/master/COPYRIGHT
// *
// * Licensed under LGPL 2.1, please see LICENSE for details
// * https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Moose.h"

namespace FluidPropertiesUtils
{
/**
 * NewtonSolve does a 1D Newton Solve to solve the equation y = f(x, z) for variable z.
 * @param[in] x constant first argument of the f(x, z) term
 * @param[in] y constant which should be equal to f(x, z) with a converged z
 * @param[in] z_initial_guess initial guess for return variables
 * @param[in] tolerance criterion on absolute difference between successive iterates to judge
 * convergence
 * @param[in] function two-variable function returning both values and derivatives as references
 * @param[in] max_its the maximum number of iterations for Newton's method
 * @return the value z such that f(x, z) = y
 */
Real NewtonSolve(const Real & x,
                 const Real & y,
                 const Real & z_initial_guess,
                 const Real & tolerance,
                 std::function<void(Real, Real, Real &, Real &, Real &)> const & func,
                 const unsigned int max_its = 100);

/**
 * NewtonSolve2D does a 2D Newton Solve to solve for the x and y such that:
 * f = func1(x, y) and g = func2(x, y). This is done for example in the constant of (v, e)
 * to (p, T) variable set conversion.
 * @param[in] f target value for func1
 * @param[in] g target value for func2
 * @param[in] x0 initial guess for first output variable
 * @param[in] y0 initial guess for second output variable
 * @param[out] x_final output for first variable
 * @param[out] y_final output for second variable
 * @param[in] tolerance criterion on absolute norm between successive iterates to judge convergence
 * @param[in] func1 two-variable function returning both values and derivatives as references
 * @param[in] func2 two-variable function returning both values and derivatives as references
 * @param[in] max_its the maximum number of iterations for Newton's method
 */
void NewtonSolve2D(const Real & f,
                   const Real & g,
                   const Real & x0,
                   const Real & y0,
                   Real & x_final,
                   Real & y_final,
                   const Real & tolerance,
                   bool & converged,
                   std::function<void(Real, Real, Real &, Real &, Real &)> const & func1,
                   std::function<void(Real, Real, Real &, Real &, Real &)> const & func2,
                   const unsigned int max_its = 100);

} // namespace FluidPropertiesUtils
