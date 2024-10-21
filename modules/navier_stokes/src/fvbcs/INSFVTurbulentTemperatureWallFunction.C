//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVTurbulentTemperatureWallFunction.h"
#include "NavierStokesMethods.h"
#include "NS.h"

registerADMooseObject("NavierStokesApp", INSFVTurbulentTemperatureWallFunction);

InputParameters
INSFVTurbulentTemperatureWallFunction::validParams()
{
  InputParameters params = FVFluxBC::validParams();

  params.addClassDescription("Adds turbulent temperature wall function.");
  params.addRequiredParam<MooseFunctorName>("T_w", "The wall temperature.");
  params.addRequiredParam<MooseFunctorName>("u", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "Density");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "Dynamic viscosity.");
  params.addRequiredParam<MooseFunctorName>(NS::cp, "The specific heat at constant pressure.");
  params.addParam<MooseFunctorName>(NS::kappa, "The thermal conductivity.");
  params.addParam<MooseFunctorName>("Pr_t", 0.58, "The turbulent Prandtl number.");
  params.addParam<MooseFunctorName>("k", "Turbulent kinetic energy functor.");
  params.deprecateParam("k", NS::TKE, "01/01/2025");
  params.addParam<Real>("C_mu", 0.09, "Coupled turbulent kinetic energy closure.");
  MooseEnum wall_treatment("eq_newton eq_incremental eq_linearized neq", "neq");
  params.addParam<MooseEnum>(
      "wall_treatment", wall_treatment, "The method used for computing the wall functions");

  params.addParam<bool>("newton_solve", false, "Whether a Newton nonlinear solve is being used");
  params.addParamNamesToGroup("newton_solve", "Advanced");
  return params;
}

INSFVTurbulentTemperatureWallFunction::INSFVTurbulentTemperatureWallFunction(
    const InputParameters & parameters)
  : FVFluxBC(parameters),
    _dim(_subproblem.mesh().dimension()),
    _T_w(getFunctor<ADReal>("T_w")),
    _u_var(getFunctor<ADReal>("u")),
    _v_var(parameters.isParamValid("v") ? &(getFunctor<ADReal>("v")) : nullptr),
    _w_var(parameters.isParamValid("w") ? &(getFunctor<ADReal>("w")) : nullptr),
    _rho(getFunctor<ADReal>(NS::density)),
    _mu(getFunctor<ADReal>(NS::mu)),
    _cp(getFunctor<ADReal>(NS::cp)),
    _kappa(getFunctor<ADReal>(NS::kappa)),
    _Pr_t(getFunctor<ADReal>("Pr_t")),
    _k(getFunctor<ADReal>(NS::TKE)),
    _C_mu(getParam<Real>("C_mu")),
    _wall_treatment(getParam<MooseEnum>("wall_treatment")),
    _newton_solve(getParam<bool>("newton_solve"))
{
}

ADReal
INSFVTurbulentTemperatureWallFunction::computeQpResidual()
{
  // Useful parameters
  const FaceInfo & fi = *_face_info;
  const Real wall_dist = std::abs((fi.elemCentroid() - fi.faceCentroid()) * fi.normal());
  const Elem & _current_elem = fi.elem();
  const auto current_argument = makeElemArg(&_current_elem);
  const auto state = determineState();
  const auto old_state = Moose::StateArg(1, Moose::SolutionIterationType::Nonlinear);
  const auto mu = _mu(current_argument, state);
  const auto rho = _rho(current_argument, state);
  const auto cp = _cp(current_argument, state);
  const auto kappa = _kappa(current_argument, state);

  // Get the velocity vector
  ADRealVectorValue velocity(_u_var(current_argument, state));
  if (_v_var)
    velocity(1) = (*_v_var)(current_argument, state);
  if (_w_var)
    velocity(2) = (*_w_var)(current_argument, state);

  // Compute the velocity and direction of the velocity component that is parallel to the wall
  const ADReal parallel_speed =
      NS::computeSpeed(velocity - velocity * (fi.normal()) * (fi.normal()));

  // Computing friction velocity and y+
  ADReal u_tau, y_plus;

  if (_wall_treatment == "eq_newton")
  {
    // Full Newton-Raphson solve to find the wall quantities from the law of the wall
    u_tau = NS::findUStar(mu, rho, parallel_speed, wall_dist);
    y_plus = wall_dist * u_tau * rho / mu;
  }
  else if (_wall_treatment == "eq_incremental")
  {
    // Incremental solve on y_plus to get the near-wall quantities
    y_plus = NS::findyPlus(mu, rho, std::max(parallel_speed, 1e-10), wall_dist);
    u_tau = parallel_speed / (std::log(std::max(NS::E_turb_constant * y_plus, 1.0 + 1e-4)) /
                              NS::von_karman_constant);
  }
  else if (_wall_treatment == "eq_linearized")
  {
    // Linearized approximation to the wall function to find the near-wall quantities faster
    const ADReal a_c = 1 / NS::von_karman_constant;
    const ADReal b_c = 1.0 / NS::von_karman_constant *
                       (std::log(NS::E_turb_constant * std::max(wall_dist, 1.0) / mu) + 1.0);
    const ADReal c_c = parallel_speed;

    u_tau = (-b_c + std::sqrt(std::pow(b_c, 2) + 4.0 * a_c * c_c)) / (2.0 * a_c);
    y_plus = wall_dist * u_tau * rho / mu;
  }
  else if (_wall_treatment == "neq")
  {
    // Assign non-equilibrium wall function value
    y_plus = wall_dist * std::sqrt(std::sqrt(_C_mu) * _k(current_argument, old_state)) * rho / mu;
    u_tau = parallel_speed / (std::log(std::max(NS::E_turb_constant * y_plus, 1.0 + 1e-4)) /
                              NS::von_karman_constant);
  }

  ADReal alpha;
  if (y_plus <= 5.0) // sub-laminar layer
  {
    alpha = kappa / (rho * cp);
  }
  else if (y_plus >= 30.0) // log-layer
  {
    const auto Pr = cp * mu / kappa;
    const auto Pr_ratio = Pr / _Pr_t(current_argument, state);
    const auto jayatilleke_P =
        9.24 * (std::pow(Pr_ratio, 0.75) - 1.0) * (1.0 + 0.28 * std::exp(-0.007 * Pr_ratio));
    const auto wall_scaling =
        1.0 / NS::von_karman_constant * std::log(NS::E_turb_constant * y_plus) + jayatilleke_P;
    alpha = u_tau * wall_dist / (_Pr_t(current_argument, state) * wall_scaling);
  }
  else // buffer layer
  {
    const auto alpha_lam = kappa / (rho * cp);
    const auto Pr = cp * mu / kappa;
    const auto Pr_t = _Pr_t(current_argument, state);
    const auto Pr_ratio = Pr / Pr_t;
    const auto jayatilleke_P =
        9.24 * (std::pow(Pr_ratio, 0.75) - 1.0) * (1.0 + 0.28 * std::exp(-0.007 * Pr_ratio));
    const auto wall_scaling =
        1.0 / NS::von_karman_constant * std::log(NS::E_turb_constant * y_plus) + jayatilleke_P;
    const auto alpha_turb = u_tau * wall_dist / (Pr_t * wall_scaling);
    const auto blending_function = (y_plus - 5.0) / 25.0;
    alpha = blending_function * alpha_turb + (1.0 - blending_function) * alpha_lam;
  }

  // To make sure new derivatives are not introduced as the solve progresses
  if (_newton_solve)
    alpha += 0 * kappa * (rho * cp) + 0 * u_tau * _Pr_t(current_argument, state) * y_plus;

  const auto face_arg = singleSidedFaceArg();
  return -rho * cp * alpha * (_T_w(face_arg, state) - _var(current_argument, state)) / wall_dist;
}
