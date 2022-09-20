//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NewtonInversion.h"
#include "MooseUtils.h"
#include "Conversion.h"
#include "Eigen/LU"

// C++ includes
#include <fstream>
#include <ctime>
#include <math.h>

namespace FluidPropertiesUtils
{
Real
NewtonSolve(const Real & x,
            const Real & y,
            const Real & z_initial_guess,
            const Real & tolerance,
            std::function<void(Real, Real, Real &, Real &, Real &)> const & func,
            const unsigned int max_its)
{
  Real next_z;
  Real f;

  // Initialize algorithm
  Real current_z = z_initial_guess;
  unsigned int iteration = 1;
  Real residual = 10;

  while (residual > tolerance)
  {
    Real new_y, df_dx, df_dz;
    func(x, current_z, new_y, df_dx, df_dz);
    f = new_y - y;
    next_z = current_z - (f / df_dz);

    // residual is absolute residual, not relative
    residual = std::abs(current_z - next_z);

    // Check for NaNs
    if (std::isnan(next_z))
      mooseError("NaN detected in Newton solve");

    current_z = next_z;
    ++iteration;

    // Check for divergence or slow convergence of Newton's method
    if (iteration > max_its)
      mooseError(
          "Newton solve convergence failed: maximum number of iterations, ", max_its, " exceeded");
  }
  return current_z;
}

void
NewtonSolve2D(const Real & f,
              const Real & g,
              const Real & x0,
              const Real & y0,
              Real & x_final,
              Real & y_final,
              const Real & tolerance,
              bool & converged,
              std::function<void(Real, Real, Real &, Real &, Real &)> const & func1,
              std::function<void(Real, Real, Real &, Real &, Real &)> const & func2,
              const unsigned int max_its)
{
  // pre-allocate working variable
  RealEigenMatrix jacobian(2, 2);
  RealEigenVector next_vec(2);
  RealEigenVector function(2);

  // initialize current_vec with initial guess
  RealEigenVector current_vec(2);
  current_vec << x0, y0;

  // values we want to reach for func1 and func2
  RealEigenVector target(2);
  target << f, g;

  // variables to keep track of algorithm progression
  converged = false;
  unsigned int iteration = 1;
  Real res1 = 1;
  Real res2 = 1;
  Real residual = 1;

  while (residual > tolerance)
  {
    Real new_f, df_dx, df_dy, new_g, dg_dx, dg_dy;
    func1(
        current_vec[0], current_vec[1], new_f, df_dx, df_dy); // get new evaluation and derivatives
    func2(
        current_vec[0], current_vec[1], new_g, dg_dx, dg_dy); // get new evaluation and derivatives
    jacobian << df_dx, df_dy,                                 // fill jacobian
        dg_dx, dg_dy;

    function << new_f, new_g;                                            // fill function
    next_vec = current_vec - (jacobian.inverse() * (function - target)); // 2D Newton Method
    res1 = (current_vec[0] - next_vec[0]);                               // update residual 1
    res2 = (current_vec[1] - next_vec[1]);                               // update residual 2
    residual = pow(pow(res1, 2) + pow(res2, 2), 0.5);                    // update residual

    // Check for nans
    if (std::isnan(next_vec[0]) || std::isnan(next_vec[1]))
    {
      x_final = current_vec[0];
      y_final = current_vec[1];
      converged = false;
      return;
    }

    current_vec = next_vec; // update current_vec for next iteration
    ++iteration;            // update iteration;

    // Check for Newton iteration not converging fast enough
    if (iteration > max_its)
    {
      x_final = current_vec[0];
      y_final = current_vec[1];
      converged = false;
      return;
    }
  }
  // save solution to x_final and y_final
  x_final = current_vec[0];
  y_final = current_vec[1];
  converged = true;
}

} // namespace FluidPropertiesUtils
