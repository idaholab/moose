//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// * This file is part of the MOOSE framework
// * https://mooseframework.inl.gov
// *
// * All rights reserved, see COPYRIGHT for full restrictions
// * https://github.com/idaholab/moose/blob/master/COPYRIGHT
// *
// * Licensed under LGPL 2.1, please see LICENSE for details
// * https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"
#include "MooseUtils.h"
#include "libmesh/dense_matrix.h"

namespace FluidPropertiesUtils
{
/**
 * NewtonSolve does a 1D Newton Solve to solve the equation y = f(x, z) for variable z.
 * @param[in] x constant first argument of the f(x, z) term
 * @param[in] y constant which should be equal to f(x, z) with a converged z
 * @param[in] z_initial_guess initial guess for return variables
 * @param[in] tolerance criterion for relative or absolute (if y is sufficiently close to zero)
 * convergence checking
 * @param[in] y_from_x_z two-variable function returning both values and derivatives as references
 * @param[in] caller_name name of the fluid properties appended to name of the routine calling the
 * method
 * @param[in] max_its the maximum number of iterations for Newton's method
 * @return a pair in which the first member is the value z such that f(x, z) = y and the second
 * member is dy/dz
 */
template <typename T, typename Functor>
std::pair<T, T>
NewtonSolve(const T & x,
            const T & y,
            const Real z_initial_guess,
            const Real tolerance,
            const Functor & y_from_x_z,
            const std::string & caller_name,
            const unsigned int max_its = 100)
{
  // R represents residual

  std::function<bool(const T &, const T &)> abs_tol_check =
      [tolerance](const T & R, const T & /*y*/)
  { return std::abs(MetaPhysicL::raw_value(R)) < tolerance; };
  std::function<bool(const T &, const T &)> rel_tol_check = [tolerance](const T & R, const T & y)
  { return std::abs(MetaPhysicL::raw_value(R / y)) < tolerance; };
  auto convergence_check = MooseUtils::absoluteFuzzyEqual(MetaPhysicL::raw_value(y), 0, tolerance)
                               ? abs_tol_check
                               : rel_tol_check;

  T z = z_initial_guess, R, new_y, dy_dx, dy_dz;
  unsigned int iteration = 0;

  using std::isnan;

  do
  {
    y_from_x_z(x, z, new_y, dy_dx, dy_dz);
    R = new_y - y;

    // We always want to perform at least one update in order to get derivatives on z correct (z
    // corresponding to the initial guess will have no derivative information), so we don't
    // immediately return if we are converged
    const bool converged = convergence_check(R, y);

#ifndef NDEBUG
    static constexpr Real perturbation_factor = 1 + 1e-8;
    T perturbed_y, dummy, dummy2;
    y_from_x_z(x, perturbation_factor * z, perturbed_y, dummy, dummy2);
    // Check the accuracy of the Jacobian
    auto J_differenced = (perturbed_y - new_y) / (1e-8 * z);
    if (!MooseUtils::relativeFuzzyEqual(J_differenced, dy_dz, 1e-2))
      mooseDoOnce(mooseWarning(caller_name + ": Bad Jacobian in NewtonSolve"));
#endif

    z += -(R / dy_dz);

    // Check for NaNs
    if (isnan(z))
      mooseException(caller_name + ": NaN detected in Newton solve");

    if (converged)
      break;
  } while (++iteration < max_its);

  // Check for divergence or slow convergence of Newton's method
  if (iteration >= max_its)
    mooseException(caller_name +
                       ": Newton solve convergence failed: maximum number of iterations, ",
                   max_its,
                   ", exceeded");

  return {z, dy_dz};
}

/**
 * NewtonSolve2D does a 2D Newton Solve to solve for the x and y such that:
 * f = f_from_x_y(x, y) and g = g_from_x_y(x, y). This is done for example in the constant of (v, e)
 * to (p, T) variable set conversion.
 * @param[in] f target value for f_from_x_y
 * @param[in] g target value for g_from_x_y
 * @param[in] x0 initial guess for first output variable
 * @param[in] y0 initial guess for second output variable
 * @param[out] x_final output for first variable
 * @param[out] y_final output for second variable
 * @param[in] f_tol criterion for relative or absolute (if f is sufficently close to zero)
 * convergence checking
 * @param[in] g_tol criterion for relative or absolute (if g is sufficently close to zero)
 * convergence checking
 * @param[in] f_from_x_y two-variable function returning both values and derivatives as references
 * @param[in] g_from_x_y two-variable function returning both values and derivatives as references
 * @param[in] max_its the maximum number of iterations for Newton's method
 * @param[in] debug whether to output the solution, residual and Jacobian on every iteration
 */
template <typename T, typename Functor1, typename Functor2>
void
NewtonSolve2D(const T & f,
              const T & g,
              const Real x0,
              const Real y0,
              T & x_final,
              T & y_final,
              const Real f_tol,
              const Real g_tol,
              const Functor1 & f_from_x_y,
              const Functor2 & g_from_x_y,
              const unsigned int max_its = 100,
              bool debug = false)
{

  constexpr unsigned int system_size = 2;
  DenseVector<T> targets = {{f, g}};
  DenseVector<Real> tolerances = {{f_tol, g_tol}};
  // R represents a residual equal to y - y_in
  auto convergence_check = [&targets, &tolerances](const auto & minus_R)
  {
    using std::abs;

    for (const auto i : index_range(minus_R))
    {
      const auto error = abs(MooseUtils::absoluteFuzzyEqual(targets(i), 0, tolerances(i))
                                 ? minus_R(i)
                                 : minus_R(i) / targets(i));
      if (error >= tolerances(i))
        return false;
    }
    return true;
  };

  DenseVector<T> u = {{x0, y0}};
  DenseVector<T> minus_R(system_size), func_evals(system_size), u_update;
  DenseMatrix<T> J(system_size, system_size);
  unsigned int iteration = 0;
#ifndef NDEBUG
  DenseVector<Real> svs(system_size), evs_real(system_size), evs_imag(system_size);
  DenseMatrix<Real> raw_J(system_size, system_size), raw_J2(system_size, system_size);
#endif

  typedef std::function<void(const T &, const T &, T &, T &, T &)> FuncType;
  std::array<FuncType, 2> func = {{f_from_x_y, g_from_x_y}};

  auto assign_solution = [&u, &x_final, &y_final]()
  {
    x_final = u(0);
    y_final = u(1);
  };
  if (debug)
    std::cout << "Target values:\n" << targets << std::endl;

  using std::isnan, std::max, std::abs;

  do
  {
    for (const auto i : make_range(system_size))
      func[i](u(0), u(1), func_evals(i), J(i, 0), J(i, 1));

    for (const auto i : make_range(system_size))
      minus_R(i) = targets(i) - func_evals(i);

    // We always want to perform at least one update in order to get derivatives on z correct (z
    // corresponding to the initial guess will have no derivative information), so we don't
    // immediately return if we are converged
    const bool converged = convergence_check(minus_R);

    // Check for NaNs before proceeding to system solve. We may simultaneously not have NaNs in z
    // but have NaNs in the function evaluation
    for (const auto i : make_range(system_size))
      if (isnan(minus_R(i)))
      {
        assign_solution();
        mooseException("NaN detected in Newton solve");
      }

    if (debug)
    {
      std::cout << "Iteration " << iteration << std::endl;
      std::cout << "Current solution vector:\n" << u << std::endl;
      std::cout << "Current (minus) residual:\n" << minus_R << std::endl;
      std::cout << "Current Jacobian:\n" << J << std::endl;
    }

    // Do some Jacobi (rowmax) preconditioning and check for an empty row
    int degenerate_row = -1;
    for (const auto i : make_range(system_size))
    {
      const auto rowmax = std::max(std::abs(J(i, 0)), std::abs(J(i, 1)));
      if (rowmax > 0)
      {
        for (const auto j : make_range(system_size))
          J(i, j) /= rowmax;
        minus_R(i) /= rowmax;
      }
      else
      {
        if (degenerate_row != -1)
          mooseException("Jacobian is all zeros in NewtonSolve2D");
        degenerate_row = i;
      }
    }

#ifndef NDEBUG
    //
    // Check nature of linearized system
    //
    for (const auto i : make_range(system_size))
      for (const auto j : make_range(system_size))
      {
        raw_J(i, j) = MetaPhysicL::raw_value(J(i, j));
        raw_J2(i, j) = MetaPhysicL::raw_value(J(i, j));
      }
    raw_J.svd(svs);
    raw_J2.evd(evs_real, evs_imag);
    if (debug)
      std::cout << "Jacobian singular values:\n" << svs << std::endl;
#endif

    if (degenerate_row == -1)
      J.lu_solve(minus_R, u_update);
    else
    {
      // use a 1D newton when the Jacobian has an empty row
      u_update(system_size - 1 - degenerate_row) =
          minus_R(system_size - 1 - degenerate_row) /
          J(system_size - 1 - degenerate_row, system_size - 1 - degenerate_row);
      u_update(degenerate_row) = 0;
    }
    // reset the decomposition
    J.zero();
    u += u_update;

    // Check for NaNs
    for (const auto i : make_range(system_size))
      if (isnan(u(i)))
      {
        assign_solution();
        mooseException("NaN detected in NewtonSolve2D");
      }

    if (converged)
      break;
  } while (++iteration < max_its);

  assign_solution();

  // Check for divergence or slow convergence of Newton's method
  if (iteration >= max_its)
    mooseException(
        "Newton solve convergence failed: maximum number of iterations, ", max_its, ", exceeded");
}
} // namespace FluidPropertiesUtils
