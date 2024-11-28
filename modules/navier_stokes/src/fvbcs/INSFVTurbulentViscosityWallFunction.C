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

registerMooseObject("NavierStokesApp", INSFVTurbulentViscosityWallFunction);

InputParameters
INSFVTurbulentViscosityWallFunction::validParams()
{
  InputParameters params = FVDirichletBCBase::validParams();
  params.addClassDescription("Adds Dirichlet BC for wall values of the turbulent viscosity.");
  params.addRequiredParam<MooseFunctorName>("u", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "Density");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "Dynamic viscosity.");
  params.addRequiredParam<MooseFunctorName>(NS::mu_t, "The turbulent viscosity.");
  params.addParam<MooseFunctorName>("k", "The turbulent kinetic energy.");
  params.deprecateParam("k", NS::TKE, "01/01/2025");
  params.addParam<Real>("C_mu", 0.09, "Coupled turbulent kinetic energy closure.");

  MooseEnum wall_treatment("eq_newton eq_incremental eq_linearized neq", "neq");
  params.addParam<MooseEnum>(
      "wall_treatment", wall_treatment, "The method used for computing the wall functions");
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
    _mu(getFunctor<ADReal>(NS::mu)),
    _mu_t(getFunctor<ADReal>(NS::mu_t)),
    _k(getFunctor<ADReal>(NS::TKE)),
    _C_mu(getParam<Real>("C_mu")),
    _wall_treatment(getParam<MooseEnum>("wall_treatment").getEnum<NS::WallTreatmentEnum>()),
    _preserve_sparsity_pattern(_fv_problem.preserveMatrixSparsityPattern())
{
}

ADReal
INSFVTurbulentViscosityWallFunction::boundaryValue(const FaceInfo & fi,
                                                   const Moose::StateArg & /* state */) const
{
  const Real wall_dist = std::abs((fi.elemCentroid() - fi.faceCentroid()) * fi.normal());
  const Elem & _current_elem = fi.elem();
  const auto current_argument = makeElemArg(&_current_elem);
  // Get the previous non linear iteration values
  const auto old_state = Moose::StateArg(1, Moose::SolutionIterationType::Nonlinear);
  const auto mu = _mu(current_argument, old_state);
  const auto rho = _rho(current_argument, old_state);

  // Get the velocity vector
  ADRealVectorValue velocity(_u_var(current_argument, old_state));
  if (_v_var)
    velocity(1) = (*_v_var)(current_argument, old_state);
  if (_w_var)
    velocity(2) = (*_w_var)(current_argument, old_state);

  // Compute the velocity and direction of the velocity component that is parallel to the wall
  const auto parallel_speed = NS::computeSpeed(velocity - velocity * (fi.normal()) * (fi.normal()));

  // Switch for determining the near wall quantities
  // wall_treatment can be: "eq_newton eq_incremental eq_linearized neq"
  ADReal y_plus;
  ADReal mut_log; // turbulent log-layer viscosity
  ADReal mu_wall; // total wall viscosity to obtain the shear stress at the wall

  if (_wall_treatment == NS::WallTreatmentEnum::EQ_NEWTON)
  {
    // Full Newton-Raphson solve to find the wall quantities from the law of the wall
    const auto u_tau = NS::findUStar(mu, rho, parallel_speed, wall_dist);
    y_plus = wall_dist * u_tau * rho / mu;
    mu_wall = rho * Utility::pow<2>(u_tau) * wall_dist / parallel_speed;
    mut_log = mu_wall - mu;
  }
  else if (_wall_treatment == NS::WallTreatmentEnum::EQ_INCREMENTAL)
  {
    // Incremental solve on y_plus to get the near-wall quantities
    y_plus = NS::findyPlus(mu, rho, std::max(parallel_speed, 1e-10), wall_dist);
    mu_wall = mu * (NS::von_karman_constant * y_plus /
                    std::log(std::max(NS::E_turb_constant * y_plus, 1 + 1e-4)));
    mut_log = mu_wall - mu;
  }
  else if (_wall_treatment == NS::WallTreatmentEnum::EQ_LINEARIZED)
  {
    // Linearized approximation to the wall function to find the near-wall quantities faster
    const ADReal a_c = 1 / NS::von_karman_constant;
    const ADReal b_c = 1 / NS::von_karman_constant *
                       (std::log(NS::E_turb_constant * std::max(wall_dist, 1.0) / mu) + 1.0);
    const ADReal c_c = parallel_speed;

    const auto u_tau = (-b_c + std::sqrt(std::pow(b_c, 2) + 4.0 * a_c * c_c)) / (2.0 * a_c);
    y_plus = wall_dist * u_tau * rho / mu;
    mu_wall = rho * Utility::pow<2>(u_tau) * wall_dist / parallel_speed;
    mut_log = mu_wall - mu;
  }
  else if (_wall_treatment == NS::WallTreatmentEnum::NEQ)
  {
    // Assign non-equilibrium wall function value
    y_plus =
        std::pow(_C_mu, 0.25) * wall_dist * std::sqrt(_k(current_argument, old_state)) * rho / mu;
    mu_wall = mu * (NS::von_karman_constant * y_plus /
                    std::log(std::max(NS::E_turb_constant * y_plus, 1.0 + 1e-4)));
    mut_log = mu_wall - mu;
  }
  else
    mooseAssert(false,
                "For `INSFVTurbulentViscosityWallFunction` , wall treatment should not reach here");

  ADReal residual = 0;
  // To keep the same sparsity pattern for all y_plus
  if (_preserve_sparsity_pattern)
    residual = 0 * mut_log * y_plus;

  if (y_plus <= 5.0)
    // sub-laminar layer
    residual += 0.0;
  else if (y_plus >= 30.0)
    // log-layer
    residual += std::max(mut_log, NS::mu_t_low_limit);
  else
  {
    // buffer layer
    const auto blending_function = (y_plus - 5.0) / 25.0;
    // the blending depends on the mut_log at y+=30
    const auto mut_log = mu * _mut_30;
    residual += std::max(blending_function * mut_log, NS::mu_t_low_limit);
  }
  return residual;
}
