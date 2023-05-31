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
  params.addRequiredCoupledVar("u", "The velocity in the x direction.");
  params.addCoupledVar("v", "The velocity in the y direction.");
  params.addCoupledVar("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>("k", "Coupled turbulent kinetic energy.");
  params.addRequiredParam<MooseFunctorName>("epsilon",
                                            "Coupled turbulent kinetic energy dissipation rate.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "Density");
  params.addRequiredParam<MooseFunctorName>("mu", "Dynamic viscosity.");
  params.addRequiredParam<MooseFunctorName>("C_mu", "Coupled turbulent kinetic energy closure.");
  params.addRequiredParam<std::vector<BoundaryName>>("walls",
                                                     "Boundaries that correspond to solid walls.");
  params.addParam<bool>(
      "linearized_yplus",
      false,
      "Boolean to indicate if yplus must be estimate locally for the blending functions.");

  params.addParam<unsigned int>(
      "n_iters_activate", 0, "Relaxation iterations after which the k-epsilon model is activated.");

  params.addParam<Real>("max_mixing_length",
                        1e10,
                        "Maximum mixing legth allowed for the domain - adjust if seeking for "
                        "realizable k-epsilon answer.");
  params.addParam<bool>(
      "wall_treatement", true, "Activate wall treatement by adding wall functions.");
  params.addParam<bool>(
      "non_equilibrium_treatement",
      false,
      "Use non-equilibrium wall treatement (faster than standard wall treatement)");
  params.addParam<Real>("rf", 1.0, "Relaxation factor.");
  params.addParam<unsigned int>(
      "iters_to_activate",
      0,
      "number of iterations needed to activate the source in the turbulent kinetic energy.");
  params.addParam<Real>("mu_t_inital", 10.0, "Initial value for the turbulent viscosity.");
  MooseEnum relaxation_method("time nl", "nl");
  params.addParam<MooseEnum>(
      "relaxation_method",
      relaxation_method,
      "The method used for relaxing the turbulent kinetic energy production. "
      "'nl' = previous nonlinear iteration and 'time' = previous timestep.");
  params.addParam<Real>("damper", 0.0, "Damping factor for nonlienar iterations.");
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
    _C_mu(getFunctor<ADReal>("C_mu")),
    _wall_boundary_names(getParam<std::vector<BoundaryName>>("walls")),
    _linearized_yplus(getParam<bool>("linearized_yplus")),
    _n_kernel_iters(0),
    _n_iters_activate(getParam<unsigned int>("n_iters_activate")),
    _max_mixing_length(getParam<Real>("max_mixing_length")),
    _wall_treatement(getParam<bool>("wall_treatement")),
    _non_equilibrium_treatement(getParam<bool>("non_equilibrium_treatement")),
    _rf(getParam<Real>("rf")),
    _iters_to_activate(getParam<unsigned int>("iters_to_activate")),
    _mu_t_inital(getParam<Real>("mu_t_inital")),
    _relaxation_method(getParam<MooseEnum>("relaxation_method")),
    _damper(getParam<Real>("damper"))
{
  _max_viscosity_value = 0.0;
}

void
kEpsilonViscosityAux::initialSetup()
{
  // Setting up limiter for turbulent viscosity
  auto current_argument = makeElemArg(_current_elem);
  const auto state = determineState();
  Real mu_t = _rho(current_argument, state).value() * _C_mu(current_argument, state).value() *
              std::pow(_k(current_argument, state).value(), 2) /
              (_epsilon(current_argument, state).value() + 1e-10);

  if (mu_t > _max_viscosity_value)
    _max_viscosity_value = std::max(mu_t, 1e3);

  for (const auto & elem : _c_fe_problem.mesh().getMesh().element_ptr_range())
  {
    _nl_damping_map[elem] = 0.0;
  }
}

ADReal
kEpsilonViscosityAux::findUStarLocalMethod(const ADReal & u, const Real & dist)
{

  /// Setting up parameters
  const auto state = determineState();
  auto rho = _rho(makeElemArg(_current_elem), state);
  auto mu = _mu(makeElemArg(_current_elem), state);
  auto nu = mu / rho;

  const ADReal a_c = 1 / _von_karman;
  const ADReal b_c = 1 / _von_karman * (std::log(_E * dist / mu) + 1.0);
  const ADReal c_c = u;
  ADReal u_tau = (-b_c + std::sqrt(std::pow(b_c, 2) + 4.0 * a_c * c_c)) / (2.0 * a_c);

  if (_linearized_yplus)
  {
    return u_tau;
  }
  else
  {
    // /// Satrting with linear guess
    // ADReal u_tau = std::sqrt(nu * u / dist);

    // Newton-Raphson method to solve for u_tau
    Real rel_err;
    for (int i = 0; i < _MAX_ITERS_U_TAU; ++i)
    {
      ADReal residual = u / u_tau - 1 / _von_karman * std::log(_E * dist * u_tau / nu);

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
                 rel_err);

    return u_tau;
  }
}

Real
kEpsilonViscosityAux::computeValue()
{

  // Boundary value
  const Elem & elem = *_current_elem;
  auto current_argument = makeElemArg(_current_elem);
  unsigned int loc_state = 0;
  const Moose::StateArg state(loc_state);
  unsigned int current_nl_iteration = static_cast<NonlinearSystemBase &>(_nl_sys)
                                          .nonlinearSolver()
                                          ->get_current_nonlinear_iteration_number();

  bool wall_bounded = false;
  Real min_wall_dist = 0.0;
  Point loc_normal;

  // if (_n_kernel_iters < (_n_iters_activate + 0)) //+2 since we need to consider assembly passes
  //   return 100.0 +
  //          _mu(current_argument, state)
  //              .value(); /// TODO: need to compute this value dynamically based in the condition
  //              number of the problem

  for (unsigned int i_side = 0; i_side < elem.n_sides(); ++i_side)
  {
    const std::vector<BoundaryID> side_bnds =
        _subproblem.mesh().getBoundaryIDs(_current_elem, i_side);

    for (const BoundaryName & name : _wall_boundary_names)
    {
      BoundaryID wall_id = _subproblem.mesh().getBoundaryID(name);
      for (BoundaryID side_id : side_bnds)
      {
        if (side_id == wall_id)
        {
          const FaceInfo * const fi = _mesh.faceInfo(&elem, i_side);
          Real dist = std::abs((fi->elemCentroid() - fi->faceCentroid()) * fi->normal());

          if (dist > min_wall_dist)
          {
            min_wall_dist = dist;
            loc_normal = fi->normal();
          }
          wall_bounded = true;
        }
      }
    }
  }

  auto time_scale = _k(current_argument, state) / _epsilon(current_argument, state);

  // auto min_constraint_time_scale =
  //     0.6 * std::sqrt(_mu(current_argument, state) / _rho(current_argument, state) /
  //     _epsilon(current_argument, state));

  // auto constraint_time_scale = std::max(min_constraint_time_scale.value(), time_scale.value());

  Real mu_t = _rho(current_argument, state).value() * _C_mu(current_argument, state).value() *
              _k(current_argument, state).value() * time_scale.value();

  Real mu_t_wall = mu_t;
  if (wall_bounded && _wall_treatement)
  {

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
      y_plus = _rho(current_argument, state) * std::pow(_C_mu(current_argument, state), 0.25) *
               std::pow(_k(current_argument, state), 0.5) * min_wall_dist /
               _mu(current_argument, state);
      auto von_karman_value = (1 / _von_karman + std::log(_E * y_plus));
      u_tau =
          std::sqrt(std::pow(_C_mu(current_argument, state), 0.25) *
                    std::pow(_k(current_argument, state), 0.5) * parallel_speed / von_karman_value);
    }
    else
    {
      u_tau = this->findUStarLocalMethod(parallel_speed, min_wall_dist);
      y_plus = min_wall_dist * u_tau * _rho(current_argument, state) / _mu(current_argument, state);
    }

    if (y_plus <= 5.0) // sub-laminar layer
    {
      mu_t_wall = _mu(current_argument, state).value();
    }
    else if (y_plus >= 30.0)
    {
      auto wall_val =
          Utility::pow<2>(u_tau) * _rho(current_argument, state) * min_wall_dist / parallel_speed;
      mu_t_wall = wall_val.value();
    }
    else
    {
      auto wall_val_log =
          Utility::pow<2>(u_tau) * _rho(current_argument, state) * min_wall_dist / parallel_speed;
      auto blending_function = (y_plus - 5.0) / 25.0;
      auto wall_val = blending_function * wall_val_log +
                      (1.0 - blending_function) * _mu(current_argument, state).value();
      mu_t_wall = wall_val.value();
    }
    mu_t = mu_t_wall;
  }

  unsigned int old_state = 2;
  Moose::SolutionIterationType type = Moose::SolutionIterationType::Time;
  if (_relaxation_method == "nl")
    type = Moose::SolutionIterationType::Nonlinear;
  const Moose::StateArg old_state_arg(old_state, type);

  auto mu_t_old = _var(current_argument, old_state_arg).value();
  mu_t = _rf * mu_t + (1.0 - _rf) * mu_t_old;

  if (current_nl_iteration == _iters_to_activate)
    _nl_damping_map[_current_elem] += 1.0;

  Real return_value;
  if ((current_nl_iteration < _iters_to_activate)) // && (!_c_fe_problem.isTransient()))
    return_value = _mu_t_inital;
  else
    return_value = std::abs(mu_t);

  if ((_relaxation_method == "nl") && (_damper > 1e-10))
    return_value += _mu_t_inital * std::exp(-_nl_damping_map[_current_elem] / _damper);

  return return_value;
}
