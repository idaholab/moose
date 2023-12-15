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
#include "NS.h"
#include "NonlinearSystemBase.h"
#include "libmesh/nonlinear_solver.h"

registerMooseObject("NavierStokesApp", kEpsilonViscosityAux);

InputParameters
kEpsilonViscosityAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Calculates the turbulent viscosity according to the k-epsilon model.");
  params.addRequiredCoupledVar("u", "The velocity in the x direction.");
  params.addCoupledVar("v", "The velocity in the y direction.");
  params.addCoupledVar("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>(NS::TKE, "Coupled turbulent kinetic energy.");
  params.addRequiredParam<MooseFunctorName>(NS::TKED,
                                            "Coupled turbulent kinetic energy dissipation rate.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "Density");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "Dynamic viscosity.");
  params.addParam<Real>("C_mu", "Coupled turbulent kinetic energy closure.");
  params.addParam<std::vector<BoundaryName>>(
      "walls", {}, "Boundaries that correspond to solid walls.");
  params.addParam<bool>(
      "linearized_yplus",
      false,
      "Boolean to indicate if yplus must be estimate locally for the blending functions.");
  params.addParam<bool>("bulk_wall_treatment", false, "Activate bulk wall treatment.");
  params.addParam<bool>(
      "non_equilibrium_treatment",
      false,
      "Use non-equilibrium wall treatement (faster than standard wall treatement)");
  return params;
}

kEpsilonViscosityAux::kEpsilonViscosityAux(const InputParameters & params)
  : AuxKernel(params),
    _dim(_subproblem.mesh().dimension()),
    _u_var(dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("u", 0))),
    _v_var(params.isParamValid("v")
               ? dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("v", 0))
               : nullptr),
    _w_var(params.isParamValid("w")
               ? dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("w", 0))
               : nullptr),
    _k(getFunctor<ADReal>(NS::TKE)),
    _epsilon(getFunctor<ADReal>(NS::TKED)),
    _rho(getFunctor<ADReal>(NS::density)),
    _mu(getFunctor<ADReal>("mu")),
    _C_mu(getParam<Real>("C_mu")),
    _wall_boundary_names(getParam<std::vector<BoundaryName>>("walls")),
    _linearized_yplus(getParam<bool>("linearized_yplus")),
    _bulk_wall_treatment(getParam<bool>("bulk_wall_treatment")),
    _non_equilibrium_treatment(getParam<bool>("non_equilibrium_treatment"))
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

ADReal
kEpsilonViscosityAux::findUStarLocalMethod(const ADReal & u, const Real & dist)
{

  /// Setting up convenient parameters
  const auto state = determineState();
  auto rho = _rho(makeElemArg(_current_elem), state);
  auto mu = _mu(makeElemArg(_current_elem), state);
  auto nu = mu / rho;

  const Real a_c = 1 / NS::von_karman_constant;
  const ADReal b_c =
      1 / NS::von_karman_constant * (std::log(NS::E_turb_constant * dist / mu) + 1.0);
  const ADReal c_c = u;

  /// Satrting with linear guess
  /// This is important to reduce the number of nonlinear iterations
  ADReal u_tau = (-b_c + std::sqrt(std::pow(b_c, 2) + 4.0 * a_c * c_c)) / (2.0 * a_c);

  if (_linearized_yplus)
    return u_tau;
  else
  {
    // Newton-Raphson method to solve for u_tau
    ADReal residual;
    for (int i = 0; i < _MAX_ITERS_U_TAU; ++i)
    {
      residual = u / u_tau -
                 1 / NS::von_karman_constant * std::log(NS::E_turb_constant * dist * u_tau / nu);

      if (residual < _REL_TOLERANCE)
        return u_tau;

      const ADReal residual_derivative =
          -1 / u_tau *
          (u / u_tau + 1 / NS::von_karman_constant * std::log(NS::E_turb_constant * dist / nu));
      const ADReal new_u_tau =
          std::max(std::numeric_limits<Real>::epsilon(), u_tau - residual / residual_derivative);
      u_tau = new_u_tau;
    }

    mooseWarning("Could not find the wall friction velocity (mu: ",
                 mu,
                 " rho: ",
                 rho,
                 " velocity: ",
                 u,
                 " wall distance: ",
                 dist,
                 ") - Relative residual: ",
                 residual.value());

    return u_tau;
  }
}

Real
kEpsilonViscosityAux::computeValue()
{
  // Convenient Arguments
  const Elem & elem = *_current_elem;
  const auto current_argument = makeElemArg(_current_elem);
  const Moose::StateArg state = determineState();

  // Determine if the element is wall bounded
  // and if bulk wall treatment needs to be activated
  const bool wall_bounded = _wall_bounded[&elem];
  Real mu_t;

  // Computing wall value for near-wall elements if bulk wall treatement is activated
  // bulk_wall_treatement should only be used for very coarse mesh problems
  if (wall_bounded && _bulk_wall_treatment)
  {
    // Computing wall value for turbulent dynamic visocisity
    const auto & elem_distances = _dist[&elem];
    const auto min_wall_distance_iterator =
        (std::min_element(elem_distances.begin(), elem_distances.end()));
    const auto min_wall_dist = *min_wall_distance_iterator;
    const size_t minIndex = std::distance(elem_distances.begin(), min_wall_distance_iterator);
    const auto loc_normal = _face_infos[&elem][minIndex]->normal();

    // Getting y_plus
    ADRealVectorValue velocity(_u_var->getElemValue(&elem, state));
    if (_v_var)
      velocity(1) = _v_var->getElemValue(&elem, state);
    if (_w_var)
      velocity(2) = _w_var->getElemValue(&elem, state);

    // Compute the velocity and direction of the velocity component that is parallel to the wall
    const ADReal parallel_speed = (velocity - velocity * loc_normal * loc_normal).norm();

    ADReal y_plus, u_tau;
    if (_non_equilibrium_treatment)
    {
      // Computing non-equilibrium nondimensional wall distance and friction velocity
      y_plus = _rho(current_argument, state) * std::pow(_C_mu, 0.25) *
               std::pow(_k(current_argument, state), 0.5) * min_wall_dist /
               _mu(current_argument, state);
      const auto von_karman_value =
          (1 / NS::von_karman_constant + std::log(NS::E_turb_constant * y_plus));
      u_tau = std::sqrt(std::pow(_C_mu, 0.25) * std::pow(_k(current_argument, state), 0.5) *
                        parallel_speed / von_karman_value);
    }
    else
    {
      u_tau = this->findUStarLocalMethod(parallel_speed, min_wall_dist);
      y_plus = min_wall_dist * u_tau * _rho(current_argument, state) / _mu(current_argument, state);
    }

    Real mu_t_wall;
    if (y_plus <= 5.0) // sub-laminar layer
      mu_t_wall = 0.0;
    else if (y_plus >= 30.0) // log-layer
    {
      const auto wall_val =
          _rho(current_argument, state) * u_tau * min_wall_dist /
              (1 / NS::von_karman_constant * std::log(NS::E_turb_constant * y_plus)) -
          _mu(current_argument, state);
      mu_t_wall = wall_val.value();
    }
    else // buffer layer
    {
      const auto wall_val_log =
          _rho(current_argument, state) * u_tau * min_wall_dist /
              (1 / NS::von_karman_constant * std::log(NS::E_turb_constant * y_plus)) -
          _mu(current_argument, state);
      const auto blending_function = (y_plus - 5.0) / 25.0;
      const auto wall_val = blending_function * wall_val_log;
      mu_t_wall = wall_val.value();
    }
    // Limiting the minimum turbulent viscosity to 1e-10
    // The reason is that smaller values can result in over dissipation
    // when solving the epsilon equation, leading to an instable coupling
    // in the k-epsilon system
    // Note: 1e-10 is 10e7 times smaller than the viscosity of water
    mu_t = std::max(mu_t_wall, 1e-10);
  }
  else
  {
    // Computing bulk value for turbulent dynamic visocisity
    const auto time_scale = _k(current_argument, state) / _epsilon(current_argument, state);
    const ADReal mu_t_nl =
        _rho(current_argument, state) * _C_mu * _k(current_argument, state) * time_scale;
    mu_t = mu_t_nl.value();
  }

  return mu_t;
}
