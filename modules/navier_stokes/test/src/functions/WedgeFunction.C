//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WedgeFunction.h"

registerMooseObject("NavierStokesTestApp", WedgeFunction);

InputParameters
WedgeFunction::validParams()
{
  InputParameters params = Function::validParams();
  params.addClassDescription("Function object for tests/ins/jeffery_hamel responsible for setting "
                             "the exact value of the velocity and pressure variables.");
  params.addRequiredParam<Real>(
      "alpha_degrees", "The wedge half-angle size (in degrees) used in computing 'f' below.");
  params.addRequiredParam<Real>("Re", "The Reynolds number used in computing 'f' below.");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "var_num",
      "var_num<3",
      "The variable (0==vel_x, 1==vel_y, 2==p) we are computing the exact solution for.");
  params.addRequiredParam<Real>("mu", "dynamic viscosity");
  params.addRequiredParam<Real>("rho", "density");
  params.addRequiredParam<Real>("K", "Constant obtained by interating the Jeffery-Hamel ODE once.");
  params.addRequiredParam<FunctionName>(
      "f", "The pre-computed semi-analytic exact solution f(theta) as a PiecewiseLinear function.");
  params.addClassDescription(
      "Function which computes the exact solution for Jeffery-Hamel flow in a wedge.");
  return params;
}

WedgeFunction::WedgeFunction(const InputParameters & parameters)
  : Function(parameters),
    FunctionInterface(this),
    _alpha_radians(libMesh::pi * (getParam<Real>("alpha_degrees") / 180.)),
    _Re(getParam<Real>("Re")),
    _var_num(getParam<unsigned int>("var_num")),
    _mu(getParam<Real>("mu")),
    _rho(getParam<Real>("rho")),
    _nu(_mu / _rho),
    _K(getParam<Real>("K")),
    _lambda(_Re * _nu / _alpha_radians),
    _p_star(-2 * _mu * _lambda * (1 + _K)),
    _f(getFunction("f"))
{
}

Real
WedgeFunction::value(Real /*t*/, const Point & p) const
{
  const Real r = std::sqrt(p(0) * p(0) + p(1) * p(1));
  const Real theta = std::atan2(p(1), p(0));

  // This is really unlikely to happen unless someone does something
  // very strange with their mesh.
  mooseAssert(r != 0., "The exact solution is singular at r=0.");

  // The "f" function is defined in terms of eta=theta/alpha, and it
  // is only defined for positive angles, since it is symmetric about
  // 0.
  const Real eta = std::abs(theta) / _alpha_radians;

  // We pass "eta" to the PiecewiseLinear function in place of "time",
  // plus a dummy Point which is not used.
  const Real f_value = _f.value(eta, _point_zero);

  // Vars 0 and 1 are the velocities.
  if (_var_num < 2)
  {
    // Converts the radial velocity vector to x and y components, respectively.
    const Real cs[2] = {std::cos(theta), std::sin(theta)};

    // Compute the centerline velocity for this r.
    const Real u_max = _lambda / r;

    // The true velocity value is simply u_max * f, times either
    // cos(theta) or sin(theta) to convert it to Cartesian coordinates.
    return u_max * f_value * cs[_var_num];
  }

  // Otherwise, we are computing the pressure.
  else
    return _p_star + (2 * _mu * _lambda) / (r * r) * (f_value + _K);
}
