//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVTurbulentViscosityWallFunctionBC.h"
#include "NavierStokesMethods.h"

registerMooseObject("NavierStokesApp", LinearFVTurbulentViscosityWallFunctionBC);

InputParameters
LinearFVTurbulentViscosityWallFunctionBC::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionBC::validParams();
  params.addClassDescription("Adds Dirichlet BC for wall values of the turbulent viscosity.");
  params.addRequiredParam<MooseFunctorName>("u", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "Density");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "Dynamic viscosity.");
  params.addParam<MooseFunctorName>("k", "The turbulent kinetic energy.");
  params.deprecateParam("k", NS::TKE, "01/01/2025");
  params.addParam<Real>("C_mu", 0.09, "Coupled turbulent kinetic energy closure.");

  MooseEnum wall_treatment("eq_newton eq_incremental eq_linearized neq", "neq");
  params.addParam<MooseEnum>(
      "wall_treatment", wall_treatment, "The method used for computing the wall functions");
  return params;
}

LinearFVTurbulentViscosityWallFunctionBC::LinearFVTurbulentViscosityWallFunctionBC(
    const InputParameters & params)
  : LinearFVAdvectionDiffusionBC(params),
    _dim(_subproblem.mesh().dimension()),
    _u_var(getFunctor<Real>("u")),
    _v_var(params.isParamValid("v") ? &(getFunctor<Real>("v")) : nullptr),
    _w_var(params.isParamValid("w") ? &(getFunctor<Real>("w")) : nullptr),
    _rho(getFunctor<Real>(NS::density)),
    _mu(getFunctor<Real>(NS::mu)),
    _k(getFunctor<Real>(NS::TKE)),
    _C_mu(getParam<Real>("C_mu")),
    _wall_treatment(getParam<MooseEnum>("wall_treatment").getEnum<NS::WallTreatmentEnum>())
{
}

Real
LinearFVTurbulentViscosityWallFunctionBC::computeTurbulentViscosity() const
{
  // Utility functions
  const auto wall_dist = computeCellToFaceDistance();
  const auto & elem = _current_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR
                          ? _current_face_info->neighborPtr()
                          : _current_face_info->elemPtr();
  const auto re = makeElemArg(elem);
  const auto old_state = Moose::StateArg(1, Moose::SolutionIterationType::Nonlinear);
  const auto mu = _mu(re, old_state);
  const auto rho = _rho(re, old_state);

  // Get the velocity vector
  RealVectorValue velocity(_u_var(re, old_state));
  if (_v_var)
    velocity(1) = (*_v_var)(re, old_state);
  if (_w_var)
    velocity(2) = (*_w_var)(re, old_state);

  // Compute the velocity and direction of the velocity component that is parallel to the wall
  const auto parallel_speed = NS::computeSpeed<Real>(
      velocity - velocity * (_current_face_info->normal()) * (_current_face_info->normal()));

  // Switch for determining the near wall quantities
  // wall_treatment can be: "eq_newton eq_incremental eq_linearized neq"
  Real y_plus = 0.0;
  Real mu_wall = 0.0; // total wall viscosity to obtain the shear stress at the wall

  if (_wall_treatment == NS::WallTreatmentEnum::EQ_NEWTON)
  {
    // Full Newton-Raphson solve to find the wall quantities from the law of the wall
    const auto u_tau = NS::findUStar<Real>(mu, rho, parallel_speed, wall_dist);
    y_plus = wall_dist * u_tau * rho / mu;
    mu_wall = rho * Utility::pow<2>(u_tau) * wall_dist / parallel_speed;
  }
  else if (_wall_treatment == NS::WallTreatmentEnum::EQ_INCREMENTAL)
  {
    // Incremental solve on y_plus to get the near-wall quantities
    y_plus = NS::findyPlus<Real>(mu, rho, std::max(parallel_speed, 1e-10), wall_dist);
    mu_wall = mu * (NS::von_karman_constant * y_plus /
                    std::log(std::max(NS::E_turb_constant * y_plus, 1 + 1e-4)));
  }
  else if (_wall_treatment == NS::WallTreatmentEnum::EQ_LINEARIZED)
  {
    // Linearized approximation to the wall function to find the near-wall quantities faster
    const Real a_c = 1 / NS::von_karman_constant;
    const Real b_c = 1 / NS::von_karman_constant *
                     (std::log(NS::E_turb_constant * std::max(wall_dist, 1.0) / mu) + 1.0);
    const Real c_c = parallel_speed;

    const auto u_tau = (-b_c + std::sqrt(std::pow(b_c, 2) + 4.0 * a_c * c_c)) / (2.0 * a_c);
    y_plus = wall_dist * u_tau * rho / mu;
    mu_wall = rho * Utility::pow<2>(u_tau) * wall_dist / parallel_speed;
  }
  else if (_wall_treatment == NS::WallTreatmentEnum::NEQ)
  {
    // Assign non-equilibrium wall function value
    y_plus = std::pow(_C_mu, 0.25) * wall_dist * std::sqrt(_k(re, old_state)) * rho / mu;
    mu_wall = mu * (NS::von_karman_constant * y_plus /
                    std::log(std::max(NS::E_turb_constant * y_plus, 1.0 + 1e-4)));
  }
  else
    mooseAssert(false,
                "For `INSFVTurbulentViscosityWallFunction` , wall treatment should not reach here");

  const Real mut_log = mu_wall - mu; // turbulent log-layer viscosity

  Real mu_t = 0;

  if (y_plus <= 5.0)
    // sub-laminar layer
    mu_t += 0.0;
  else if (y_plus >= 30.0)
    // log-layer
    mu_t += std::max(mut_log, NS::mu_t_low_limit);
  else
  {
    // buffer layer
    const auto blending_function = (y_plus - 5.0) / 25.0;
    // the blending depends on the mut_log at y+=30
    const auto mut_log = mu * _mut_30;
    mu_t += std::max(blending_function * mut_log, NS::mu_t_low_limit);
  }
  return mu_t;
}

Real
LinearFVTurbulentViscosityWallFunctionBC::computeBoundaryValue() const
{
  return this->computeTurbulentViscosity();
}

Real
LinearFVTurbulentViscosityWallFunctionBC::computeBoundaryNormalGradient() const
{
  const auto elem_arg = makeElemArg(_current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                                        ? _current_face_info->elemPtr()
                                        : _current_face_info->neighborPtr());
  const Real distance = computeCellToFaceDistance();
  return (this->computeTurbulentViscosity() - raw_value(_var(elem_arg, determineState()))) /
         distance;
}

Real
LinearFVTurbulentViscosityWallFunctionBC::computeBoundaryValueMatrixContribution() const
{
  mooseError("We should not solve for the turbulent viscosity directly meaning that this should "
             "contribute to neither vector nor a right hand side.");
}

Real
LinearFVTurbulentViscosityWallFunctionBC::computeBoundaryValueRHSContribution() const
{
  mooseError("We should not solve for the turbulent viscosity directly meaning that this should "
             "contribute to neither vector nor a right hand side.");
}

Real
LinearFVTurbulentViscosityWallFunctionBC::computeBoundaryGradientMatrixContribution() const
{
  mooseError("We should not solve for the turbulent viscosity directly meaning that this should "
             "contribute to neither vector nor a right hand side.");
}

Real
LinearFVTurbulentViscosityWallFunctionBC::computeBoundaryGradientRHSContribution() const
{
  mooseError("We should not solve for the turbulent viscosity directly meaning that this should "
             "contribute to neither vector nor a right hand side.");
}
