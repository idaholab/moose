/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "WedgeFunction.h"

template<>
InputParameters validParams<WedgeFunction>()
{
  InputParameters params = validParams<Function>();
  params.addRequiredParam<Real>("alpha_degrees", "The wedge half-angle size (in degrees) used in computing 'f' below.");
  params.addRequiredParam<Real>("Re", "The Reynolds number used in computing 'f' below.");
  params.addRequiredParam<unsigned int>("component", "The component (x==0, y==1) of the velocity we are computing.");
  params.addRequiredParam<Real>("mu", "dynamic viscosity");
  params.addRequiredParam<Real>("rho", "density");
  params.addRequiredParam<FunctionName>("f", "The pre-computed semi-analytic exact solution f(theta) as a PiecewiseLinear function.");
  params.addClassDescription("Function representing the exact solution for Jeffery-Hamel flow in a wedge.");
  return params;
}

WedgeFunction::WedgeFunction(const InputParameters & parameters) :
    Function(parameters),
    FunctionInterface(this),
    _alpha_radians(libMesh::pi * (getParam<Real>("alpha_degrees") / 180.)),
    _Re(getParam<Real>("Re")),
    _component(getParam<unsigned int>("component")),
    _mu(getParam<Real>("mu")),
    _rho(getParam<Real>("rho")),
    _f(getFunction("f"))
{
}

Real
WedgeFunction::value(Real /*t*/, const Point & p)
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
  const Real f_value = _f.value(eta, Point(0., 0., 0.));

  // Converts the radial velocity vector to x and y components, respectively.
  const Real cs[2] = {std::cos(theta), std::sin(theta)};

  // We need to compute u_max for the users given Re.  Since
  //
  // Re := (u_max * r * alpha) / nu
  //
  // we can rearrange this to get:
  //
  // u_max = (Re * nu) / (r * alpha).
  //
  // Note that u_max is *not* a constant, it depends on r!  However,
  // we have the relationship:
  //
  // r * u_max = const.
  //
  // so it makes sense to use this definition for Re.  Consequently,
  // the u_max we are computing here is for the current value of r, so
  // we don't have to divide by r later when computing the velocity
  // magnitude to return.
  const Real nu = _mu / _rho;
  const Real u_max = _Re * nu / (r * _alpha_radians);

  // The true velocity value is simply u_max * f, times either
  // cos(theta) or sin(theta) to convert it to Cartesian coordinates.
  return u_max * f_value * cs[_component];
}
