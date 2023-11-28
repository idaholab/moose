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
  params.addRequiredParam<MooseFunctorName>("k", "Coupled turbulent kinetic energy.");
  params.addRequiredParam<MooseFunctorName>("epsilon",
                                            "Coupled turbulent kinetic energy dissipation rate.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "Density");
  params.addRequiredParam<MooseFunctorName>("mu", "Dynamic viscosity.");
  params.addParam<Real>("C_mu", "Coupled turbulent kinetic energy closure.");
  params.addParam<std::vector<BoundaryName>>(
      "walls", {}, "Boundaries that correspond to solid walls.");
  params.addParam<bool>(
      "linearized_yplus",
      false,
      "Boolean to indicate if yplus must be estimate locally for the blending functions.");
  params.addParam<bool>("bulk_wall_treatment", false, "Activate bulk wall treatment.");
  params.addParam<bool>(
      "non_equilibrium_treatement",
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
    _k(getFunctor<ADReal>("k")),
    _epsilon(getFunctor<ADReal>("epsilon")),
    _rho(getFunctor<ADReal>(NS::density)),
    _mu(getFunctor<ADReal>("mu")),
    _C_mu(getParam<Real>("C_mu")),
    _wall_boundary_names(getParam<std::vector<BoundaryName>>("walls")),
    _linearized_yplus(getParam<bool>("linearized_yplus")),
    _bulk_wall_treatment(getParam<bool>("bulk_wall_treatment")),
    _non_equilibrium_treatement(getParam<bool>("non_equilibrium_treatement"))
{
}

void
kEpsilonViscosityAux::initialSetup()
{
  if (!_wall_boundary_names.empty())
  {
    _wall_bounded = NS::getWallBoundedElements(_wall_boundary_names, _c_fe_problem, _subproblem);
    _normal = NS::getElementFaceNormal(_wall_boundary_names, _c_fe_problem, _subproblem);
    _dist = NS::getWallDistance(_wall_boundary_names, _c_fe_problem, _subproblem);
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

  const ADReal a_c = 1 / _von_karman;
  const ADReal b_c = 1 / _von_karman * (std::log(_E * dist / mu) + 1.0);
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
      residual = u / u_tau - 1 / _von_karman * std::log(_E * dist * u_tau / nu);

      if (residual < _REL_TOLERANCE)
        return u_tau;

      ADReal residual_derivative =
          -1 / u_tau * (u / u_tau + 1 / _von_karman * std::log(_E * dist / nu));
      ADReal new_u_tau = std::max(1e-20, u_tau - residual / residual_derivative);
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
  auto current_argument = makeElemArg(_current_elem);
  const Moose::StateArg state = determineState();

  // Determine if the element is wall bounded
  // and if bulk wall treatment needs to be activated
  bool wall_bounded = _wall_bounded[&elem];
  Real mu_t;

  // Computing wall value for near-wall elements if bulk wall treatement is activated
  // bulk_wall_treatement should only be used for very coarse mesh problems
  if (wall_bounded && _bulk_wall_treatment)
  {

    // Computing wall value for turbulent dynamic visocisity

    auto min_wall_distance_iterator = (std::min_element(_dist[&elem].begin(), _dist[&elem].end()));
    auto min_wall_dist = *min_wall_distance_iterator;
    size_t minIndex = std::distance(_dist[&elem].begin(), min_wall_distance_iterator);
    auto loc_normal = _normal[&elem][minIndex];

    // Getting y_plus
    ADRealVectorValue velocity(_u_var->getElemValue(&elem, state));
    if (_v_var)
      velocity(1) = _v_var->getElemValue(&elem, state);
    if (_w_var)
      velocity(2) = _w_var->getElemValue(&elem, state);

    // Compute the velocity and direction of the velocity component that is parallel to the wall
    ADReal parallel_speed = (velocity - velocity * loc_normal * loc_normal).norm();

    ADReal y_plus, u_tau;
    if (_non_equilibrium_treatement)
    {
      // Computing non-equilibrium nondimensional wall distance and friction velocity
      y_plus = _rho(current_argument, state) * std::pow(_C_mu, 0.25) *
               std::pow(_k(current_argument, state), 0.5) * min_wall_dist /
               _mu(current_argument, state);
      auto von_karman_value = (1 / _von_karman + std::log(_E * y_plus));
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
    {
      mu_t_wall = 0.0;
    }
    else if (y_plus >= 30.0) // log-layer
    {
      auto wall_val = _rho(current_argument, state) * u_tau * min_wall_dist /
                          (1 / _von_karman * std::log(_E * y_plus)) -
                      _mu(current_argument, state);
      mu_t_wall = wall_val.value();
    }
    else // buffer layer
    {
      auto wall_val_log = _rho(current_argument, state) * u_tau * min_wall_dist /
                              (1 / _von_karman * std::log(_E * y_plus)) -
                          _mu(current_argument, state);
      auto blending_function = (y_plus - 5.0) / 25.0;
      auto wall_val = blending_function * wall_val_log;
      mu_t_wall = wall_val.value();
    }

    mu_t = std::max(mu_t_wall, 1e-10);
  }
  else
  {
    // Computing bulk value for turbulent dynamic visocisity
    auto time_scale = _k(current_argument, state) / _epsilon(current_argument, state);
    ADReal mu_t_nl =
        _rho(current_argument, state) * _C_mu * _k(current_argument, state) * time_scale;
    mu_t = mu_t_nl.value();
  }

  return mu_t;
}
