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
#include "CurvatureCorrec.h"

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
  params.addRequiredParam<MooseFunctorName>(NS::cp, "The spcific heat at constant pressure.");
  params.addParam<MooseFunctorName>(NS::kappa, "The thermal conductivity.");
  params.addParam<MooseFunctorName>("Pr_t", 0.58, "The turbulent Prandtl number.");
  params.addRequiredParam<MooseFunctorName>(NS::TKE, "The turbulent kinetic energy.");
  params.addParam<Real>("C_mu", 0.09, "Coupled turbulent kinetic energy closure.");
  MooseEnum wall_treatment("eq_newton eq_incremental eq_linearized neq", "neq");
  params.addParam<MooseEnum>("wall_treatment",
                             wall_treatment,
                             "The method used for computing the wall functions "
                             "'eq_newton', 'eq_incremental', 'eq_linearized', 'neq'");
    // Wall function correction parameters
  params.addParam<Real>("rough_ks", 0, "equivalent sand roughness height");
  params.addParam<MooseFunctorName>(NS::curv_R, 0, "curvature ");
  params.addParam<bool>("convex", false, "is the mesh convex ?");
  params.addParam<MooseFunctorName>(
      "x_curvature_axis", 0, "x coordinate of the axis along which the curvature is");
  params.addParam<MooseFunctorName>(
      "y_curvature_axis", 0, "y coordinate of the axis along which the curvature is");
  params.addParam<MooseFunctorName>(
      "z_curvature_axis", 0, "z coordinate of the axis along which the curvature is");
  params.addParamNamesToGroup(
      "rough_ks convex x_curvature_axis y_curvature_axis z_curvature_axis",
      "Wall function correction parameters");
  return params;
}

INSFVTurbulentTemperatureWallFunction::INSFVTurbulentTemperatureWallFunction(
    const InputParameters & params)
  : FVFluxBC(params),
    _dim(_subproblem.mesh().dimension()),
    _T_w(getFunctor<ADReal>("T_w")),
    _u_var(getFunctor<ADReal>("u")),
    _v_var(params.isParamValid("v") ? &(getFunctor<ADReal>("v")) : nullptr),
    _w_var(params.isParamValid("w") ? &(getFunctor<ADReal>("w")) : nullptr),
    _rho(getFunctor<ADReal>(NS::density)),
    _mu(getFunctor<ADReal>(NS::mu)),
    _cp(getFunctor<ADReal>(NS::cp)),
    _kappa(getFunctor<ADReal>(NS::kappa)),
    _Pr_t(getFunctor<ADReal>("Pr_t")),
    _k(getFunctor<ADReal>(NS::TKE)),
    _C_mu(getParam<Real>("C_mu")),
    _wall_treatment(getParam<MooseEnum>("wall_treatment")),
    _rough_ks(getParam<Real>("rough_ks")),
    _convex(getParam<bool>("convex")),
    _x_curvature_axis(params.isParamValid("x_curvature_axis")
                          ? &getFunctor<ADReal>("x_curvature_axis")
                          : nullptr),
    _y_curvature_axis(params.isParamValid("y_curvature_axis")
                          ? &getFunctor<ADReal>("y_curvature_axis")
                          : nullptr),
    _z_curvature_axis(
        params.isParamValid("z_curvature_axis") ? &getFunctor<ADReal>("z_curvature_axis") : nullptr)
{
  if (!(params.isParamValid(NS::curv_R)) && params.isParamValid("x_curvature_axis"))
    paramError("x_curvature_axis",
               "Curvature in the `x` direction provided but wall curvature corrections are not "
               "active since ",
               NS::curv_R,
               " has not been provided");

  if (!(params.isParamValid(NS::curv_R)) && params.isParamValid("y_curvature_axis"))
    paramError("y_curvature_axis",
               "Curvature in the `y` direction provided but wall curvature corrections are not "
               "active since ",
               NS::curv_R,
               " has not been provided");

  if (!(params.isParamValid(NS::curv_R)) && params.isParamValid("z_curvature_axis"))
    paramError("z_curvature_axis",
               "Curvature in the `z` direction provided but wall curvature corrections are not "
               "active since ",
               NS::curv_R,
               " has not been provided");

  if (params.isParamValid(NS::curv_R) &&
      !(params.isParamValid("x_curvature_axis") && params.isParamValid("y_curvature_axis") &&
        params.isParamValid("z_curvature_axis")))
    mooseError("When curvature correction is active, all `x_curvature_axis`, `y_curvature_axis`, "
               "`z_curvature_axis` curvature axis need to be provided");
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
  const ADReal parallel_speed = (velocity - velocity * (fi.normal()) * (fi.normal())).norm();

  // Computing friction velocity and y+
  ADReal u_tau, y_plus, q_tau;

  if (_wall_treatment == "eq_newton")
  {
    ADReal y_plus = 0; 
    if (_curv_R)
    {
      ADReal speed_u = parallel_speed; // streamwise speed
      ADReal speed_w = 0;              // swirling speed
      // Getting curvature corrections
      if (_x_curvature_axis || _y_curvature_axis || _z_curvature_axis)
      {
        const auto x_curvature_axis = (*_x_curvature_axis)(makeElemArg(&_current_elem), state);
        const auto y_curvature_axis = (*_y_curvature_axis)(makeElemArg(&_current_elem), state);
        const auto z_curvature_axis = (*_z_curvature_axis)(makeElemArg(&_current_elem), state);
        if (x_curvature_axis > 1e-12 || y_curvature_axis > 1e-12 || y_curvature_axis > 1e-12)
        {
          ADRealVectorValue curv_axis(x_curvature_axis, y_curvature_axis, z_curvature_axis);
          ADRealVectorValue w_vector =
            fi.normal().cross(curv_axis) / fi.normal().cross(curv_axis).norm();
          speed_u =
            (velocity - velocity * fi.normal() * fi.normal() - velocity * w_vector * w_vector)
                  .norm();
          speed_w = (velocity * w_vector * w_vector).norm();
        }
      }

      // Get new stemwise friction velocity with swirling corrections
      u_tau = NS::findUStar(mu, rho, speed_u, wall_dist, _rough_ks);

      // Get swirling friction velocity
      ADReal w_tau = 0.0;
      const auto curv_R = (*_curv_R)(current_argument, state);
      if (curv_R > 1e-12)
        w_tau = CurvatureCorrec::findWStar(
          mu.value(), rho.value(), speed_w, wall_dist, curv_R, _convex);
      y_plus = (wall_dist * std::sqrt(u_tau*u_tau + w_tau*w_tau) * rho / mu).value();
      q_tau = std::sqrt(u_tau*u_tau + w_tau*w_tau);
    }
    else
    {
      // Full Newton-Raphson solve to find the wall quantities from the law of the wall
      u_tau = NS::findUStar(mu, rho, parallel_speed, wall_dist, _rough_ks);
      q_tau = u_tau;
      y_plus = (wall_dist * u_tau * rho / mu).value();
    }
  }
  else if (_wall_treatment == "eq_incremental")
  {
    // Incremental solve on y_plus to get the near-wall quantities
    y_plus = NS::findyPlus(mu, rho, std::max(parallel_speed, 1e-10), wall_dist);
    u_tau = parallel_speed / (std::log(std::max(NS::E_turb_constant * y_plus, 1.0 + 1e-4)) /
                              NS::von_karman_constant);
    q_tau = u_tau;
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
    q_tau = u_tau;
  }
  else if (_wall_treatment == "neq")
  {
    // Assign non-equilibrium wall function value
    y_plus = wall_dist * std::sqrt(std::sqrt(_C_mu) * _k(current_argument, old_state)) * rho / mu;
    u_tau = parallel_speed / (std::log(std::max(NS::E_turb_constant * y_plus, 1.0 + 1e-4)) /
                              NS::von_karman_constant);
    q_tau = u_tau;
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
    ADReal jayatilleke_P =
        9.24 * (std::pow(Pr_ratio, 0.75) - 1.0) * (1.0 + 0.28 * std::exp(-0.007 * Pr_ratio));
    const ADReal ks_plus = _rough_ks * q_tau * rho / mu;
    ADReal speed_norm;
    if (ks_plus > 1e-12)
    {
      jayatilleke_P = 3.15 * std::pow(Pr, 0.695) * std::pow(((1+NS::C_rough_constant*ks_plus)/NS::E_turb_constant - 1/NS::E_turb_constant), 0.359) +
                      pow(1/(1+NS::C_rough_constant*ks_plus), 0.6) * jayatilleke_P;
    }
    const auto wall_scaling =
        1/NS::von_karman_constant * std::log(NS::E_turb_constant * y_plus/(1+NS::C_rough_constant*ks_plus))  /q_tau + jayatilleke_P;
    alpha = q_tau * wall_dist / (_Pr_t(current_argument, state) * wall_scaling);
  }
  else // buffer layer
  {
    const auto alpha_lam = kappa / (rho * cp);
    const auto Pr = cp * mu / kappa;
    const auto Pr_ratio = Pr / _Pr_t(current_argument, state);
    ADReal jayatilleke_P =
        9.24 * (std::pow(Pr_ratio, 0.75) - 1.0) * (1.0 + 0.28 * std::exp(-0.007 * Pr_ratio));
    const ADReal ks_plus = _rough_ks * q_tau * rho / mu;
    ADReal speed_norm;
    if (ks_plus > 1e-12)
    {
      jayatilleke_P = 3.15 * std::pow(Pr, 0.695) * std::pow(((1+NS::C_rough_constant*ks_plus)/NS::E_turb_constant - 1/NS::E_turb_constant), 0.359) +
                      pow(1/(1+NS::C_rough_constant*ks_plus), 0.6) * jayatilleke_P;
    }
    const auto wall_scaling =
        1/NS::von_karman_constant * std::log(NS::E_turb_constant * y_plus/(1+NS::C_rough_constant*ks_plus))  /q_tau + jayatilleke_P;
    const auto alpha_turb = q_tau * wall_dist / (_Pr_t(current_argument, state) * wall_scaling);
    const auto blending_function = (y_plus - 5.0) / 25.0;
    alpha = blending_function * alpha_turb + (1.0 - blending_function) * alpha_lam;
  }

  const auto face_arg = singleSidedFaceArg();
  return -rho * cp * alpha * (_T_w(face_arg, state) - _var(current_argument, state)) / wall_dist;
}
