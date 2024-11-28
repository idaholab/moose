//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVTKEDWallFunctionBC.h"
#include "Function.h"
#include "NavierStokesMethods.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", INSFVTKEDWallFunctionBC);

InputParameters
INSFVTKEDWallFunctionBC::validParams()
{
  InputParameters params = FVDirichletBCBase::validParams();
  params.addClassDescription("Adds Reichardt extrapolated wall values to set up directly the"
                             "Dirichlet BC for the turbulent kinetic energy dissipation rate.");
  params.addRequiredParam<MooseFunctorName>("u", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "Density");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "Dynamic viscosity.");
  params.addRequiredParam<MooseFunctorName>(NS::mu_t, "The turbulent viscosity.");
  params.addRequiredParam<MooseFunctorName>("k", "The turbulent kinetic energy.");
  params.deprecateParam("k", NS::TKE, "01/01/2025");
  params.addParam<MooseFunctorName>("C_mu", 0.09, "Coupled turbulent kinetic energy closure.");
  params.addParam<bool>("newton_solve", false, "Whether a Newton nonlinear solve is being used");
  params.addParamNamesToGroup("newton_solve", "Advanced");
  return params;
}

INSFVTKEDWallFunctionBC::INSFVTKEDWallFunctionBC(const InputParameters & params)
  : FVDirichletBCBase(params),
    _dim(_subproblem.mesh().dimension()),
    _u_var(getFunctor<ADReal>("u")),
    _v_var(params.isParamValid("v") ? &(getFunctor<ADReal>("v")) : nullptr),
    _w_var(params.isParamValid("w") ? &(getFunctor<ADReal>("w")) : nullptr),
    _rho(getFunctor<ADReal>(NS::density)),
    _mu(getFunctor<ADReal>(NS::mu)),
    _mu_t(getFunctor<ADReal>(NS::mu_t)),
    _k(getFunctor<ADReal>(NS::TKE)),
    _C_mu(getFunctor<ADReal>("C_mu")),
    _newton_solve(getParam<bool>("newton_solve"))
{
}

ADReal
INSFVTKEDWallFunctionBC::boundaryValue(const FaceInfo & fi, const Moose::StateArg & state) const
{
  const Real dist = std::abs((fi.elemCentroid() - fi.faceCentroid()) * fi.normal());
  const Elem & _current_elem = fi.elem();
  const auto mu = _mu(makeElemArg(&_current_elem), state);
  const auto rho = _rho(makeElemArg(&_current_elem), state);

  // Assign boundary weights to element
  // This is based on the theory of linear TKE development for each boundary
  // This is, it assumes no interaction across turbulence production from boundaries
  Real weight = 0.0;
  for (unsigned int i_side = 0; i_side < _current_elem.n_sides(); ++i_side)
    weight += static_cast<Real>(_subproblem.mesh().getBoundaryIDs(&_current_elem, i_side).size());

  // Get the velocity vector
  ADRealVectorValue velocity(_u_var(makeElemArg(&_current_elem), state));
  if (_v_var)
    velocity(1) = (*_v_var)(makeElemArg(&_current_elem), state);
  if (_w_var)
    velocity(2) = (*_w_var)(makeElemArg(&_current_elem), state);

  // Compute the velocity and direction of the velocity component that is parallel to the wall
  const ADReal parallel_speed =
      NS::computeSpeed(velocity - velocity * (fi.normal()) * (fi.normal()));

  // Get friction velocity
  const ADReal u_star = NS::findUStar(mu, rho, parallel_speed, dist);

  // Get associated non-dimensional wall distance
  const ADReal y_plus = dist * u_star * rho / mu;

  const auto TKE = _k(makeElemArg(&_current_elem), state);

  if (y_plus <= 5.0) // sub-laminar layer
  {
    const auto laminar_value = 2.0 * weight * TKE * mu / std::pow(dist, 2);
    if (!_newton_solve)
      return laminar_value;
    else
      // Additional zero term to make sure new derivatives are not introduced as y_plus
      // changes
      return laminar_value + 0 * _mu_t(makeElemArg(&_current_elem), state);
  }
  else if (y_plus >= 30.0) // log-layer
  {
    const auto turbulent_value = weight * _C_mu(makeElemArg(&_current_elem), state) *
                                 std::pow(std::abs(TKE), 1.5) /
                                 (_mu_t(makeElemArg(&_current_elem), state) * dist);
    if (!_newton_solve)
      return turbulent_value;
    else
      // Additional zero term to make sure new derivatives are not introduced as y_plus changes
      return turbulent_value + 0 * mu;
  }
  else // blending function
  {
    const auto laminar_value = 2.0 * weight * TKE * mu / std::pow(dist, 2);
    const auto turbulent_value = weight * _C_mu(makeElemArg(&_current_elem), state) *
                                 std::pow(std::abs(TKE), 1.5) /
                                 (_mu_t(makeElemArg(&_current_elem), state) * dist);
    const auto interpolation_coef = (y_plus - 5.0) / 25.0;
    return (interpolation_coef * (turbulent_value - laminar_value) + laminar_value);
  }
}
