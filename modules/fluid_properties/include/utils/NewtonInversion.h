// * This file is part of the MOOSE framework
// * https://www.mooseframework.org
// *
// * All rights reserved, see COPYRIGHT for full restrictions
// * https://github.com/idaholab/moose/blob/master/COPYRIGHT
// *
// * Licensed under LGPL 2.1, please see LICENSE for details
// * https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/utility.h"
#include <functional>

namespace NewtonMethod
{
  /**
   * NewtonSolve does a 1D Newton Solve to find z from x and y.
   * @param[in] x input variable one
   * @param[in] y input variable two
   * @param[in] z_initial_guess initial guess for return variables
   * @param[in] tolerance parameter defined in input file. Defaults to 1e-8
   * @param[in] lambda fcn for finding derivatives of y wrt x & z
   */
  Real NewtonSolve(const Real & x,
                   const Real & y,
                   const Real & z_initial_guess,
                   const Real & tolerance,
                   std::function<void(Real, Real, Real &, Real &, Real &)> const & func);

  /**
  * NewtonSolve does a 1D Newton Solve to find z from x and y.
  * @param[in] f input variable one
  * @param[in] g input variable two
  * @param[in] x0 initial guess for first output variable
  * @param[in] y0 initial guess for second output variable
  * @param[out] x_final output for first variable
  * @param[out] y_final output for second variable
  * @param[in] tolerance parameter defined in input file. Defaults to 1e-8
  * @param[in] lambda fcn for finding derivatives of f wrt x & y
  * @param[in] lambda fcn for finding derivatives of g wrt x & y
  */
  void NewtonSolve2D(const Real & f,
                     const Real & g,
                     const Real & x0,
                     const Real & y0,
                     Real & x_final,
                     Real & y_final,
                     const Real & tolerance,
                     std::function<void(Real, Real, Real &, Real &, Real &)> const & func1,
                     std::function<void(Real, Real, Real &, Real &, Real &)> const & func2);

} //namespace NewtonMethod
