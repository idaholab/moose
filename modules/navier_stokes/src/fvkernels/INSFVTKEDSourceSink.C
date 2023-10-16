//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVTKEDSourceSink.h"
#include "NS.h"
#include "NonlinearSystemBase.h"
#include "NavierStokesMethods.h"
#include "libmesh/nonlinear_solver.h"

registerMooseObject("NavierStokesApp", INSFVTKEDSourceSink);

InputParameters
INSFVTKEDSourceSink::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription("Elental kernel to compute the production and destruction "
                             " terms of turbulent kinetic energy disspation (TKED).");
  params.addRequiredCoupledVar("u", "The velocity in the x direction.");
  params.addCoupledVar("v", "The velocity in the y direction.");
  params.addCoupledVar("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>("k", "Coupled turbulent kinetic energy.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "fluid density");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "Dynamic viscosity.");
  params.addRequiredParam<MooseFunctorName>("mu_t", "Turbulent viscosity.");
  params.addParam<std::vector<BoundaryName>>("walls", "Boundaries that correspond to solid walls.");
  params.addRequiredParam<MooseFunctorName>("C1_eps", "First epsilon coefficient");
  params.addRequiredParam<MooseFunctorName>("C2_eps", "Second epsilon coefficient");
  params.addParam<Real>("max_mixing_length",
                        10.0,
                        "Maximum mixing legth allowed for the domain - adjust for realizable "
                        "k-epsilon to work properly.");
  params.addParam<bool>(
      "linearized_model", false, "Boolean to determine if the problem is linearized.");
  params.addParam<MooseFunctorName>(
      "linear_variable", 1.0, "Linearization coefficient in case the problem has been linearized.");
  params.addParam<bool>(
      "realizable_constraint",
      true,
      "Boolean to determine if the kEpsilon mixing length realizability constrints are applied.");
  params.set<unsigned short>("ghost_layers") = 2;
  params.addParam<Real>("rf", 1.0, "Relaxation factor.");
  params.addParam<bool>(
      "non_equilibrium_treatement",
      true,
      "Use non-equilibrium wall treatement (faster than standard wall treatement)");
  params.addParam<Real>("C_mu", 0.09, "Coupled turbulent kinetic energy closure.");
  MooseEnum turbulence_type("time nl", "nl");
  params.addParam<MooseEnum>(
      "relaxation_method",
      turbulence_type,
      "The method used for relaxing the turbulent kinetic energy production. "
      "'nl' = previous nonlinear iteration and 'time' = previous timestep.");
  params.addParam<unsigned int>(
      "iters_to_activate",
      5,
      "number of iterations needed to activate the source in the turbulent kinetic energy.");
  params.addParam<Real>(
      "top_production_bound", 100.0, "Top scale bound for turbulent kinetic energy production.");
  params.addParam<Real>(
      "top_destruction_bound", 100.0, "Top scale bound for turbulent kinetic energy destruction.");
  return params;
}

INSFVTKEDSourceSink::INSFVTKEDSourceSink(const InputParameters & params)
  : FVElementalKernel(params),
    _dim(_subproblem.mesh().dimension()),
    _u_var(dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("u", 0))),
    _v_var(params.isParamValid("v")
               ? dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("v", 0))
               : nullptr),
    _w_var(params.isParamValid("w")
               ? dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("w", 0))
               : nullptr),
    _k(getFunctor<ADReal>("k")),
    _rho(getFunctor<ADReal>(NS::density)),
    _mu(getFunctor<ADReal>(NS::mu)),
    _mu_t(getFunctor<ADReal>("mu_t")),
    _wall_boundary_names(getParam<std::vector<BoundaryName>>("walls")),
    _C1_eps(getFunctor<ADReal>("C1_eps")),
    _C2_eps(getFunctor<ADReal>("C2_eps")),
    _max_mixing_length(getParam<Real>("max_mixing_length")),
    _linearized_model(getParam<bool>("linearized_model")),
    _linear_variable(getFunctor<ADReal>("linear_variable")),
    _realizable_constraint(getParam<bool>("realizable_constraint")),
    _rf(getParam<Real>("rf")),
    _non_equilibrium_treatement(getParam<bool>("non_equilibrium_treatement")),
    _C_mu(getParam<Real>("C_mu")),
    _relaxation_method(getParam<MooseEnum>("relaxation_method")),
    _iters_to_activate(getParam<unsigned int>("iters_to_activate")),
    _top_production_bound(getParam<Real>("top_production_bound")),
    _top_destruction_bound(getParam<Real>("top_destruction_bound"))
{

  if (!_u_var)
    paramError("u", "the u velocity must be an INSFVVelocityVariable.");

  if (_dim >= 2 && !_v_var)
    paramError("v",
               "In two or more dimensions, the v velocity must be supplied and it must be an "
               "INSFVVelocityVariable.");

  if (_dim >= 3 && !_w_var)
    paramError("w",
               "In three-dimensions, the w velocity must be supplied and it must be an "
               "INSFVVelocityVariable.");

  _loc_dt = _dt;
  _stored_time = _fe_problem.time();

  _wall_bounded = *(NS::getWallBoundedElements(_wall_boundary_names, _fe_problem, _subproblem));
  _normal = *(NS::getElementFaceNormal(_wall_boundary_names, _fe_problem, _subproblem));
  _dist = *(NS::getWallDistance(_wall_boundary_names, _fe_problem, _subproblem));

  for (const auto & elem : _fe_problem.mesh().getMesh().element_ptr_range())
  {
    _symmetric_strain_tensor_norm_old[elem] = 0.0;
    _old_destruction[elem] = 0.0;
    _pevious_nl_sol[elem] = 0.0;
    _production_NL_old[elem] = 0.0;
    _destruction_NL_old[elem] = 0.0;
    _pevious_production[elem] = 0.0;
    _pevious_destruction[elem] = 0.0;
  }
}

ADReal
INSFVTKEDSourceSink::computeQpResidual()
{

  ADReal residual = 0.0;
  ADReal production = 0.0;
  ADReal destruction = 0.0;
  ADReal production_old_time = 0.0;
  ADReal destruction_old_time = 0.0;
  unsigned int time_state = 0;
  const Moose::StateArg state(time_state);
  const bool _relax_on_nonlinear = (_relaxation_method == "nl");
  unsigned int current_nl_iteration = static_cast<NonlinearSystemBase &>(_sys)
                                          .nonlinearSolver()
                                          ->get_current_nonlinear_iteration_number();

  if (_wall_bounded[_current_elem])
  {
    std::vector<ADReal> u_tau_vec, u_tau_vec_old;
    Real tot_weight = 0.0;

    ADRealVectorValue velocity(_u_var->getElemValue(_current_elem, state));
    if (_v_var)
      velocity(1) = _v_var->getElemValue(_current_elem, state);
    if (_w_var)
      velocity(2) = _w_var->getElemValue(_current_elem, state);

    for (unsigned int i = 0; i < _normal[_current_elem].size(); i++)
    {
      auto parallel_speed =
          (velocity - velocity * _normal[_current_elem][i] * _normal[_current_elem][i]).norm();
      auto distance = _dist[_current_elem][i];
      auto loc_u_tau = NS::findUStar(_mu(makeElemArg(_current_elem), state),
                                     _rho(makeElemArg(_current_elem), state),
                                     std::max(parallel_speed, 1e-10),
                                     distance);
      u_tau_vec.push_back(loc_u_tau);
      tot_weight += 1.0;
    }

    Real eq_value = 0.0;
    for (unsigned int i = 0; i < u_tau_vec.size(); i++)
    {
      auto kp = std::pow(u_tau_vec[i], 2) / std::sqrt(_C_mu) / tot_weight;
      eq_value += std::pow(kp.value(), 0.5) * std::pow(_C_mu, 0.75) /
                  (_von_karman * _dist[_current_elem][0]);
    }

    // eq_value = _rf * eq_value + (1.0 - _rf) * _pevious_production[_current_elem];
    _pevious_production[_current_elem] = eq_value;

    residual = _var(makeElemArg(_current_elem), state) - eq_value;
  }
  else
  {

    if (((_fe_problem.time() > _stored_time) && (_dt >= _loc_dt)) || _relax_on_nonlinear)
    {

      unsigned int loc_time_state = 0;
      Moose::SolutionIterationType type = Moose::SolutionIterationType::Time;
      if (_relax_on_nonlinear)
      {
        loc_time_state = 1;
        type = Moose::SolutionIterationType::Nonlinear;
      }
      const Moose::StateArg loc_state(loc_time_state, type);

      const auto & grad_u = _u_var->adGradSln(_current_elem, determineState());
      auto Sij_00 = grad_u(0);
      ADReal symmetric_strain_tensor_sq_norm = Utility::pow<2>(Sij_00);
      if (_dim >= 2)
      {
        const auto & grad_v = _v_var->adGradSln(_current_elem, determineState());
        auto Sij_01 = 0.5 * (grad_u(1) + grad_v(0));
        auto Sij_11 = grad_v(1);
        symmetric_strain_tensor_sq_norm += 2.0 * Utility::pow<2>(Sij_01) + Utility::pow<2>(Sij_11);
        if (_dim >= 3)
        {
          const auto & grad_w = _w_var->adGradSln(_current_elem, determineState());
          auto Sij_02 = 0.5 * (grad_u(2) + grad_w(0));
          auto Sij_12 = 0.5 * (grad_v(2) + grad_w(1));
          auto Sij_22 = grad_w(2);
          symmetric_strain_tensor_sq_norm += 2.0 * Utility::pow<2>(Sij_02) +
                                             2.0 * Utility::pow<2>(Sij_12) +
                                             Utility::pow<2>(Sij_22);
        }
      }

      auto production_k = 2.0 * _mu_t(makeElemArg(_current_elem), determineState()) *
                          symmetric_strain_tensor_sq_norm;

      auto time_scale = _k(makeElemArg(_current_elem), determineState()) /
                            (_var(makeElemArg(_current_elem), determineState()) + 1e-10) +
                        1e-10;

      production =
          _C1_eps(makeElemArg(_current_elem), determineState()) * production_k / time_scale;
      destruction = _C2_eps(makeElemArg(_current_elem), determineState()) *
                    _rho(makeElemArg(_current_elem), determineState()) *
                    _var(makeElemArg(_current_elem), determineState()) / time_scale;

      // production = _rf * production + (1.0 - _rf) * _pevious_production[_current_elem];
      // destruction = _rf * destruction + (1.0 - _rf) * _pevious_destruction[_current_elem];

      // _pevious_production[_current_elem] = production.value();
      // _pevious_destruction[_current_elem] = destruction.value();

      // if (std::abs(_pevious_production[_current_elem]) >
      //     _top_production_bound * std::abs(_pevious_destruction[_current_elem]))
      //   _pevious_production[_current_elem] =
      //       _top_production_bound * std::abs(_pevious_destruction[_current_elem]) *
      //       _pevious_production[_current_elem] / std::abs(_pevious_production[_current_elem]);

      // if (std::abs(_pevious_destruction[_current_elem]) >
      //     _top_destruction_bound * std::abs(_pevious_production[_current_elem]))
      //   _pevious_destruction[_current_elem] =
      //       _top_destruction_bound * std::abs(_pevious_production[_current_elem]) *
      //       _pevious_destruction[_current_elem] / std::abs(_pevious_destruction[_current_elem]);

      // _loc_dt = _dt;
      // _stored_time = _fe_problem.time();
    }

    // residual = _pevious_destruction[_current_elem] - _pevious_production[_current_elem];
    // if (_fe_problem.isTransient())
    //   residual +=
    //       _rho(makeElemArg(_current_elem), state) * _var.dot(makeElemArg(_current_elem), state);

    residual = destruction - production;
  }

  _pevious_nl_sol[_current_elem] = _rf * _var(makeElemArg(_current_elem), state).value() +
                                   (1.0 - _rf) * _pevious_nl_sol[_current_elem];
  _production_NL_old[_current_elem] = production.value();
  _destruction_NL_old[_current_elem] = destruction.value();

  // return residual;
  // if ((current_nl_iteration < _iters_to_activate))
  //   return 0.0;
  // else
  //   return residual;

  return residual;
}
