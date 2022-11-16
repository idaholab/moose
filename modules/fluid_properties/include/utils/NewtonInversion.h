// * This file is part of the MOOSE framework
// * https://www.mooseframework.org
// *
// * All rights reserved, see COPYRIGHT for full restrictions
// * https://github.com/idaholab/moose/blob/master/COPYRIGHT
// *
// * Licensed under LGPL 2.1, please see LICENSE for details
// * https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"
#include "MooseUtils.h"

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
 * @return a pair in which the first member is the value z such that f(x, z) = y and the second
 * member is dy/dz
 */
template <typename T, typename Functor>
std::pair<T, T>
NewtonSolve(const T & x,
            const T & y,
            const Real z_initial_guess,
            const Real tolerance,
            const Functor & func,
            const unsigned int max_its = 100)
{
  std::function<bool(const T &, const T &)> abs_tol_check =
      [tolerance](const T & f, const T & /*y*/)
  { return MetaPhysicL::raw_value(std::abs(f)) < tolerance; };
  std::function<bool(const T &, const T &)> rel_tol_check = [tolerance](const T & f, const T & y)
  { return MetaPhysicL::raw_value(std::abs(f / y)) < tolerance; };
  auto convergence_check = MetaPhysicL::raw_value(y) == 0 ? abs_tol_check : rel_tol_check;

  T z = z_initial_guess, f, new_y, dy_dx, dy_dz;
  unsigned int iteration = 0;

  do
  {
    func(x, z, new_y, dy_dx, dy_dz);
    f = new_y - y;

    if (convergence_check(f, y))
      break;

#ifdef DEBUG
    static constexpr Real perturbation_factor = 1 + 1e-8;
    T perturbed_y, dummy, dummy2;
    func(x, perturbation_factor * z, perturbed_y, dummy, dummy2);
    // Check the accuracy of the Jacobian
    auto J_differenced = (perturbed_y - new_y) / (1e-8 * z);
    if (!MooseUtils::relativeFuzzyEqual(J_differenced, dy_dz, 1e-4))
      mooseError("Bad Jacobian in NewtonSolve");
#endif

    z += -(f / dy_dz);

    // Check for NaNs
    if (std::isnan(z))
      mooseError("NaN detected in Newton solve");
  } while (++iteration < max_its);

  // Check for divergence or slow convergence of Newton's method
  if (iteration >= max_its)
    mooseError(
        "Newton solve convergence failed: maximum number of iterations, ", max_its, " exceeded");

  return {z, dy_dz};
}

template <typename T, typename Functor>
std::pair<T, T>
NewtonSolve(const DenseVector<T> & y_in,
            const DenseVector<Real> z_initial_guess,
            const DenseVector<Real> tolerance,
            const std::array<Functor, 2> & func,
            const unsigned int max_its = 100)
{
  mooseAssert(y_in.size() == z_initial_guess.size(), "Must be the same size");
  mooseAssert(y_in.size() == tolerance.size(), "Must be the same size");
  mooseAssert(y_in.size() == func.size(), "Must be the same size");

  constexpr unsigned int system_size = 2;
  // R represents a residual equal to y - y_in
  auto convergence_check = [&y_in, &tolerance](const auto & minus_R)
  {
    for (const auto i : index_range(minus_R))
    {
      const auto error = std::abs(y_in(i) == 0 ? minus_R(i) : minus_R(i) / y_in(i));
      if (error >= tolerance(i))
        return false;
    }
    return true;
  };

  DenseVector<T> z = z_initial_guess;
  DenseVector<T> minus_R(system_size), y(system_size), z_update;
  DenseMatrix<T> J(system_size, system_size);
  std::array<std::array<T, 2>, 2> dy_dz;
  unsigned int iteration = 0;

  do
  {
    for (const auto i : make_range(system_size))
      func[i](z(0), z(1), y(i), J(i, 0), J(i, 1));

    for (const auto i : make_range(system_size))
      minus_R(i) = y_in(i) - y(i);

    if (convergence_check(minus_R))
      break;

    J.lu_solve(minus_R, z_update);
    z += z_update;

    // Check for NaNs
    for (const auto i : make_range(system_size))
      if (std::isnan(z(i)))
        mooseError("NaN detected in Newton solve");
  } while (++iteration < max_its);

  // Check for divergence or slow convergence of Newton's method
  if (iteration >= max_its)
    mooseError(
        "Newton solve convergence failed: maximum number of iterations, ", max_its, " exceeded");

  return {z(0), z(1)};
}

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
