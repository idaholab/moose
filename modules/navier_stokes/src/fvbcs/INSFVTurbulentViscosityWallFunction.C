//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVTurbulentViscosityWallFunction.h"
#include "Function.h"
#include "NavierStokesMethods.h"

registerMooseObject("MooseApp", INSFVTurbulentViscosityWallFunction);

InputParameters
INSFVTurbulentViscosityWallFunction::validParams()
{
  InputParameters params = FVDirichletBCBase::validParams();
  params.addClassDescription("Adds Reichardt extrapolated wall values to set up directly the"
                             "Dirichlet BC for the turbulent kinetic energy.");
  params.addRequiredParam<MooseFunctorName>("u", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "Density");
  params.addRequiredParam<MooseFunctorName>("mu", "Dynamic viscosity.");
  params.addRequiredParam<MooseFunctorName>("mu_t", "The turbulent viscosity.");
  params.addRequiredParam<MooseFunctorName>("k", "The turbulent kinetic energy.");
  params.addParam<MooseFunctorName>("C_mu", 0.09, "Coupled turbulent kinetic energy closure.");
  params.addParam<bool>(
      "linearized_yplus",
      false,
      "Boolean to indicate if yplus must be estimate locally for the blending functions.");
  params.addParam<Real>("min_mixing_length",
                        1.0,
                        "Maximum mixing legth allowed for the domain - adjust if seeking for "
                        "realizable k-epsilon answer.");
  params.addParam<bool>(
      "non_equilibrium_treatement",
      false,
      "Use non-equilibrium wall treatement (faster than standard wall treatement)");
  return params;
}

INSFVTurbulentViscosityWallFunction::INSFVTurbulentViscosityWallFunction(
    const InputParameters & params)
  : FVDirichletBCBase(params),
    _dim(_subproblem.mesh().dimension()),
    _u_var(getFunctor<ADReal>("u")),
    _v_var(params.isParamValid("v") ? &(getFunctor<ADReal>("v")) : nullptr),
    _w_var(params.isParamValid("w") ? &(getFunctor<ADReal>("w")) : nullptr),
    _rho(getFunctor<ADReal>(NS::density)),
    _mu(getFunctor<ADReal>("mu")),
    _mu_t(getFunctor<ADReal>("mu_t")),
    _k(getFunctor<ADReal>("k")),
    _C_mu(getFunctor<ADReal>("C_mu")),
    _linearized_yplus(getParam<bool>("linearized_yplus")),
    _min_mixing_length(getParam<Real>("min_mixing_length")),
    _non_equilibrium_treatement(getParam<bool>("non_equilibrium_treatement"))
{
}

ADReal
INSFVTurbulentViscosityWallFunction::boundaryValue(const FaceInfo & fi) const
{
  Real wall_dist = std::abs((fi.elemCentroid() - fi.faceCentroid()) * fi.normal());
  const Elem & _current_elem = fi.elem();
  auto current_argument = makeElemArg(&_current_elem);
  const auto state = determineState();

  // Assign boundary weights to element
  Real weight = 0.0;
  for (unsigned int i_side = 0; i_side < _current_elem.n_sides(); ++i_side)
  {
    weight += static_cast<Real>(_subproblem.mesh().getBoundaryIDs(&_current_elem, i_side).size());
  }
  //_console << "Weight: " << weight << std::endl;

  // Get the velocity vector
  ADRealVectorValue velocity(_u_var(current_argument, state));
  if (_v_var)
    velocity(1) = (*_v_var)(current_argument, state);
  if (_w_var)
    velocity(2) = (*_w_var)(current_argument, state);

  // Compute the velocity and direction of the velocity component that is parallel to the wall
  ADReal parallel_speed = (velocity - velocity * (fi.normal()) * (fi.normal())).norm();

  ADReal u_tau;
  if (_linearized_yplus)
  {
    constexpr Real karman_cte = 0.4187;
    constexpr Real E = 9.793;
    const ADReal a_c = 1 / karman_cte;
    const ADReal b_c =
        1 / karman_cte * (std::log(E * wall_dist / _mu(current_argument, state)) + 1.0);
    const ADReal c_c = parallel_speed;
    u_tau = (-b_c + std::sqrt(std::pow(b_c, 2) + 4.0 * a_c * c_c)) / (2.0 * a_c);
  }
  else
    u_tau = NS::findUStar(
        _mu(current_argument, state), _rho(current_argument, state), parallel_speed, wall_dist);

  if (_non_equilibrium_treatement)
    u_tau = std::pow(_C_mu(current_argument, state), -0.25) *
            std::pow(_k(current_argument, state), -0.5) * std::pow(u_tau, 2);

  ADReal y_plus = wall_dist * u_tau * _rho(current_argument, state) / _mu(current_argument, state);

  if (y_plus <= 5.0) // sub-laminar layer
  {
    return _mu(current_argument, state);
  }
  else if (y_plus >= 30.0)
  {
    auto wall_val =
        Utility::pow<2>(u_tau) * _rho(current_argument, state) * wall_dist / parallel_speed;
    return wall_val + _mu(current_argument, state);
  }
  else
  {
    auto wall_val_log =
        Utility::pow<2>(u_tau) * _rho(current_argument, state) * wall_dist / parallel_speed +
        _mu(current_argument, state);
    auto blending_function = (y_plus - 5.0) / 25.0;
    auto wall_val =
        blending_function * wall_val_log + (1.0 - blending_function) * _mu(current_argument, state);
    return wall_val;
  }
}
