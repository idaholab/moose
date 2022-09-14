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

// This file performs a Newton solve in 1D and 2D. In 1D, solve fails to converge
// and error outputs if it takes more than 100 iterations. In 2D, if more than 100
// iterations occur, save results references and set convergence to false. Applications
// using 2D Newton Solve will need to write routines to take care of values where this
// occurs. See TabulatedBicubicFluidProperties for example (routine checkOutofBounds).

namespace NewtonMethod
{
Real
NewtonSolve(const Real & x,
            const Real & y,
            const Real & z_initial_guess,
            const Real & tolerance,
            std::function<void(Real, Real, Real &, Real &, Real &)> const & func)
{
  Real current_z =
      z_initial_guess; // find good initial guess, know something about e from interpolation tables
  Real next_z;
  Real f;
  unsigned int iteration = 1;
  Real residual = current_z;

  while (residual > tolerance)
  {
    Real new_y, df_dx, df_dz;
    func(x, current_z, new_y, df_dx, df_dz);
    f = new_y - y;
    next_z = current_z - (f / df_dz);
    residual = std::abs(current_z - next_z);
    // Check for NaNs
    if (std::isnan(next_z))
      mooseError("NaN detected in Newton solve");

    current_z = next_z;
    ++iteration;

    // Check for divergence or slow convergence of Newton's method
    if (iteration > 100)
      mooseError("Convergence Failed in Newton Solve");
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
              std::function<void(Real, Real, Real &, Real &, Real &)> const & func2)
{
  converged = false;
  RealEigenMatrix jacobian(2, 2); // Compute Jacobian

  RealEigenVector current_vec(2); // initialize current_vec with initial guess
  current_vec << x0, y0;

  RealEigenVector next_vec(2); // initialize "next" vector

  RealEigenVector target(2); // Real h and Real s
  target << f, g;

  RealEigenVector function(2);

  unsigned int iteration = 1;
  Real res1 = 1; // initialize residual;
  Real res2 = 1;
  Real residual = 1;

  while (residual > tolerance) // iterate until residual is smaller than tolerance reached
  {
    Real new_f, df_dx, df_dy, new_g, dg_dx, dg_dy;
    func1(current_vec[0], current_vec[1], new_f, df_dx, df_dy); // get new h and derivatives
    func2(current_vec[0], current_vec[1], new_g, dg_dx, dg_dy); // get new s and derivatives
    jacobian << df_dx, df_dy,                                   // fill jacobian
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
    if (iteration > 100)
    {
      x_final = current_vec[0];
      y_final = current_vec[1];
      converged = false;
      return;
    }
  }
  // save solution to x_final and y_final
  x_final = current_vec[0]; // returned p
  y_final = current_vec[1]; // returned y
  converged = true;         // convergence successful
}

} // namespace NewtonMethod
