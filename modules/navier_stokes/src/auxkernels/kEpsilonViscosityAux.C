//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "kEpsilonViscosityAux.h"
#include "NavierStokesMethods.h"
#include "NonlinearSystemBase.h"
#include "libmesh/nonlinear_solver.h"

registerMooseObject("NavierStokesApp", kEpsilonViscosityAux);

InputParameters
kEpsilonViscosityAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Calculates the turbulent viscosity according to the k-epsilon model.");
  params.addRequiredParam<MooseFunctorName>("u", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>("k", "Coupled turbulent kinetic energy.");
  params.deprecateParam("k", NS::TKE, "01/01/2025");
  params.addRequiredParam<MooseFunctorName>(NS::TKED,
                                            "Coupled turbulent kinetic energy dissipation rate.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "Density");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "Dynamic viscosity.");
  params.addParam<Real>("C_mu", "Coupled turbulent kinetic energy closure.");
  params.addParam<Real>("mu_t_ratio_max", 1e5, "Maximum allowable mu_t_ratio : mu/mu_t.");
  params.addParam<std::vector<BoundaryName>>(
      "walls", {}, "Boundaries that correspond to solid walls.");
  params.addParam<bool>("bulk_wall_treatment", false, "Activate bulk wall treatment.");
  MooseEnum wall_treatment("eq_newton eq_incremental eq_linearized neq", "eq_newton");
  params.addParam<MooseEnum>("wall_treatment",
                             wall_treatment,
                             "The method used for computing the wall functions."
                             "'eq_newton', 'eq_incremental', 'eq_linearized', 'neq'");
  MooseEnum scale_limiter("none standard", "standard");
  params.addParam<MooseEnum>("scale_limiter",
                             scale_limiter,
                             "The method used to limit the k-epsilon time scale."
                             "'none', 'standard'");
  params.addParam<bool>("newton_solve", false, "Whether a Newton nonlinear solve is being used");
  params.addParamNamesToGroup("newton_solve", "Advanced");

  return params;
}

kEpsilonViscosityAux::kEpsilonViscosityAux(const InputParameters & params)
  : AuxKernel(params),
    _dim(_subproblem.mesh().dimension()),
    _u_var(getFunctor<ADReal>("u")),
    _v_var(params.isParamValid("v") ? &(getFunctor<ADReal>("v")) : nullptr),
    _w_var(params.isParamValid("w") ? &(getFunctor<ADReal>("w")) : nullptr),
    _k(getFunctor<ADReal>(NS::TKE)),
    _epsilon(getFunctor<ADReal>(NS::TKED)),
    _rho(getFunctor<ADReal>(NS::density)),
    _mu(getFunctor<ADReal>(NS::mu)),
    _C_mu(getParam<Real>("C_mu")),
    _mu_t_ratio_max(getParam<Real>("mu_t_ratio_max")),
    _wall_boundary_names(getParam<std::vector<BoundaryName>>("walls")),
    _bulk_wall_treatment(getParam<bool>("bulk_wall_treatment")),
    _wall_treatment(getParam<MooseEnum>("wall_treatment").getEnum<NS::WallTreatmentEnum>()),
    _scale_limiter(getParam<MooseEnum>("scale_limiter")),
    _newton_solve(getParam<bool>("newton_solve"))
{
}

void
kEpsilonViscosityAux::initialSetup()
{
  if (!_wall_boundary_names.empty())
  {
    NS::getWallBoundedElements(
        _wall_boundary_names, _c_fe_problem, _subproblem, blockIDs(), _wall_bounded);
    NS::getWallDistance(_wall_boundary_names, _c_fe_problem, _subproblem, blockIDs(), _dist);
    NS::getElementFaceArgs(
        _wall_boundary_names, _c_fe_problem, _subproblem, blockIDs(), _face_infos);
  }
}

Real
kEpsilonViscosityAux::computeValue()
{
  // Convenient Arguments
  const Elem & elem = *_current_elem;
  const auto current_argument = makeElemArg(_current_elem);
  const Moose::StateArg state = determineState();
  const auto rho = _rho(makeElemArg(_current_elem), state);
  const auto mu = _mu(makeElemArg(_current_elem), state);
  const auto nu = mu / rho;

  // Determine if the element is wall bounded
  // and if bulk wall treatment needs to be activated
  const bool wall_bounded = _wall_bounded.find(_current_elem) != _wall_bounded.end();
  Real mu_t;

  // Computing wall value for near-wall elements if bulk wall treatment is activated
  // bulk_wall_treatment should only be used for very coarse mesh problems
  if (wall_bounded && _bulk_wall_treatment)
  {
    // Computing wall value for turbulent dynamic viscosity
    const auto & elem_distances = _dist[&elem];
    const auto min_wall_distance_iterator =
        (std::min_element(elem_distances.begin(), elem_distances.end()));
    const auto min_wall_dist = *min_wall_distance_iterator;
    const size_t minIndex = std::distance(elem_distances.begin(), min_wall_distance_iterator);
    const auto loc_normal = _face_infos[&elem][minIndex]->normal();

    // Getting y_plus
    ADRealVectorValue velocity(_u_var(current_argument, state));
    if (_v_var)
      velocity(1) = (*_v_var)(current_argument, state);
    if (_w_var)
      velocity(2) = (*_w_var)(current_argument, state);

    // Compute the velocity and direction of the velocity component that is parallel to the wall
    const auto parallel_speed = NS::computeSpeed(velocity - velocity * loc_normal * loc_normal);

    // Switch for determining the near wall quantities
    // wall_treatment can be: "eq_newton eq_incremental eq_linearized neq"
    ADReal y_plus;
    ADReal mut_log; // turbulent log-layer viscosity
    ADReal mu_wall; // total wall viscosity to obtain the shear stress at the wall

    if (_wall_treatment == NS::WallTreatmentEnum::EQ_NEWTON)
    {
      // Full Newton-Raphson solve to find the wall quantities from the law of the wall
      const auto u_tau = NS::findUStar(mu, rho, parallel_speed, min_wall_dist);
      y_plus = min_wall_dist * u_tau * rho / mu;
      mu_wall = rho * Utility::pow<2>(u_tau) * min_wall_dist / parallel_speed;
      mut_log = mu_wall - mu;
    }
    else if (_wall_treatment == NS::WallTreatmentEnum::EQ_INCREMENTAL)
    {
      // Incremental solve on y_plus to get the near-wall quantities
      y_plus = NS::findyPlus(mu, rho, std::max(parallel_speed, 1e-10), min_wall_dist);
      mu_wall = mu * (NS::von_karman_constant * y_plus /
                      std::log(std::max(NS::E_turb_constant * y_plus, 1 + 1e-4)));
      mut_log = mu_wall - mu;
    }
    else if (_wall_treatment == NS::WallTreatmentEnum::EQ_LINEARIZED)
    {
      // Linearized approximation to the wall function to find the near-wall quantities faster
      const ADReal a_c = 1 / NS::von_karman_constant;
      const ADReal b_c = 1 / NS::von_karman_constant *
                         (std::log(NS::E_turb_constant * std::max(min_wall_dist, 1.0) / mu) + 1.0);
      const ADReal c_c = parallel_speed;

      const auto u_tau = (-b_c + std::sqrt(std::pow(b_c, 2) + 4.0 * a_c * c_c)) / (2.0 * a_c);
      y_plus = min_wall_dist * u_tau * rho / mu;
      mu_wall = rho * Utility::pow<2>(u_tau) * min_wall_dist / parallel_speed;
      mut_log = mu_wall - mu;
    }
    else if (_wall_treatment == NS::WallTreatmentEnum::NEQ)
    {
      // Assign non-equilibrium wall function value
      y_plus = min_wall_dist * std::sqrt(std::sqrt(_C_mu) * _k(current_argument, state)) * rho / mu;
      mu_wall = mu * (NS::von_karman_constant * y_plus /
                      std::log(std::max(NS::E_turb_constant * y_plus, 1 + 1e-4)));
      mut_log = mu_wall - mu;
    }
    else
      mooseAssert(false, "For `kEpsilonViscosityAux` , wall treatment should not reach here");

    if (y_plus <= 5.0)
      // sub-laminar layer
      mu_t = 0.0;
    else if (y_plus >= 30.0)
      // log-layer
      mu_t = std::max(mut_log.value(), NS::mu_t_low_limit);
    else
    {
      // buffer layer
      const auto blending_function = (y_plus - 5.0) / 25.0;
      // the blending depends on the mut_log at y+=30
      const auto mut_log = mu * (NS::von_karman_constant * 30.0 /
                                     std::log(std::max(NS::E_turb_constant * 30.0, 1 + 1e-4)) -
                                 1.0);
      mu_t = std::max(raw_value(blending_function * mut_log), NS::mu_t_low_limit);
    }
  }
  else
  {
    ADReal time_scale;
    if (_scale_limiter == "standard")
    {
      time_scale = std::max(_k(current_argument, state) / _epsilon(current_argument, state),
                            std::sqrt(nu / _epsilon(current_argument, state)));
    }
    else
    {
      time_scale = _k(current_argument, state) / _epsilon(current_argument, state);
    }
    // For newton solvers, epsilon might not be bounded
    if (_newton_solve)
      time_scale = _k(current_argument, state) /
                   std::max(NS::epsilon_low_limit, _epsilon(current_argument, state));

    const ADReal mu_t_nl =
        _rho(current_argument, state) * _C_mu * _k(current_argument, state) * time_scale;
    mu_t = mu_t_nl.value();
    if (_newton_solve)
      mu_t = std::max(mu_t, NS::mu_t_low_limit);
  }
  // Turbulent viscosity limiter
  return std::min(mu_t, _mu_t_ratio_max * raw_value(mu));
}
