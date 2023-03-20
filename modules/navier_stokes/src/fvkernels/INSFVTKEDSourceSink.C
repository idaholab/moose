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
    _C_mu(getParam<Real>("C_mu"))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("INSFV is not supported by local AD indexing. In order to use INSFV, please run the "
             "configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif

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
  for (const auto & elem : _fe_problem.mesh().getMesh().element_ptr_range())
  {
    _symmetric_strain_tensor_norm_old[elem] = 0.0;
    _old_destruction[elem] = 0.0;
    _pevious_nl_sol[elem] = 0.0;
    _production_NL_old[elem] = 0.0;
    _destruction_NL_old[elem] = 0.0;
    _pevious_production[elem] = 0.0;
    _pevious_destruction[elem] = 0.0;

    auto wall_bounded = false;
    for (unsigned int i_side = 0; i_side < elem->n_sides(); ++i_side)
    {
      const std::vector<BoundaryID> side_bnds = _subproblem.mesh().getBoundaryIDs(elem, i_side);
      for (const BoundaryName & name : _wall_boundary_names)
      {
        BoundaryID wall_id = _subproblem.mesh().getBoundaryID(name);
        for (BoundaryID side_id : side_bnds)
        {
          if (side_id == wall_id)
          {
            const FaceInfo * const fi = _mesh.faceInfo(elem, i_side);
            Real dist = std::abs((fi->elemCentroid() - fi->faceCentroid()) * fi->normal());
            _dist[elem].push_back(dist);
            _normal[elem].push_back(fi->normal());
            wall_bounded = true;
          }
        }
      }
    }
    _wall_bounded[elem] = wall_bounded;
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

  if (_wall_bounded[_current_elem])
  {
    std::vector<ADReal> u_tau_vec, u_tau_vec_old;
    Real tot_weight = 0.0;
    // if (_non_equilibrium_treatement)
    // {
    //   u_tau_vec.push_back(std::pow(_C_mu, 0.25) * std::pow(_var(makeElemArg(_current_elem)),
    //   0.5)); u_tau_vec_old.push_back(std::pow(_C_mu, 0.25) *
    //                           std::pow(_var(makeElemArg(_current_elem), 1), 0.5));
    //   tot_weight += 1.0;
    // }
    // else
    // {
    // Getting y_plus
    ADRealVectorValue velocity(_u_var->getElemValue(_current_elem));
    if (_v_var)
      velocity(1) = _v_var->getElemValue(_current_elem);
    if (_w_var)
      velocity(2) = _w_var->getElemValue(_current_elem);

    for (unsigned int i = 0; i < _normal[_current_elem].size(); i++)
    {
      auto parallel_speed =
          (velocity - velocity * _normal[_current_elem][i] * _normal[_current_elem][i]).norm();
      auto distance = _dist[_current_elem][i];
      auto loc_u_tau = NS::findUStar(_mu(makeElemArg(_current_elem)),
                                     _rho(makeElemArg(_current_elem)),
                                     std::max(parallel_speed, 1e-10),
                                     distance);
      u_tau_vec.push_back(loc_u_tau);
      tot_weight += 1.0;
    }
    // }

    Real eq_value = 0.0;
    for (unsigned int i = 0; i < u_tau_vec.size(); i++)
    {
      // auto tau_w_loc = _rho(makeElemArg(_current_elem)) * std::pow(u_tau_vec[i], 2);
      // auto rho_loc = _rho(makeElemArg(_current_elem));
      // auto non_eq_tau_w_loc =
      //     std::pow(_C_mu, 0.25) * std::pow(_var(makeElemArg(_current_elem)), 0.5);
      // auto scaled_distance_loc = _von_karman * _dist[_current_elem][i];
      // production +=
      //     std::pow(tau_w_loc, 2) / (tot_weight * rho_loc * non_eq_tau_w_loc *
      //     scaled_distance_loc);
      // destruction += std::pow(non_eq_tau_w_loc, 3) / scaled_distance_loc;

      // auto tau_w_loc_old = _rho(makeElemArg(_current_elem)) * std::pow(u_tau_vec_old[i], 2);
      // auto rho_loc_old = _rho(makeElemArg(_current_elem), 1);
      // auto non_eq_tau_w_loc_old =
      //     std::pow(_C_mu, 0.25) * std::pow(_var(makeElemArg(_current_elem), 1), 0.5);
      // production_old_time +=
      //     std::pow(tau_w_loc_old, 2) /
      //     (tot_weight * rho_loc_old * non_eq_tau_w_loc_old * scaled_distance_loc);
      // destruction_old_time += std::pow(non_eq_tau_w_loc_old, 3) / scaled_distance_loc;

      auto kp = std::pow(u_tau_vec[i], 2) / std::sqrt(_C_mu) / tot_weight;
      eq_value += std::pow(kp.value(), 1.5) * std::pow(_C_mu, 0.75) /
                  (_von_karman * _dist[_current_elem][0]);
    }

    // production = _rf * production + (1.0 - _rf) * production_old_time;
    // destruction = _rf * destruction + (1.0 - _rf) * destruction_old_time;
    // residual = destruction - production;

    residual = _var(makeElemArg(_current_elem)) - eq_value;
  }
  else
  {
    // constexpr Real offset = 0.0; // prevents explosion of sqrt(x) derivative to infinity

    if (_loc_dt != _dt)
    {

      const auto & grad_u = _u_var->adGradSln(_current_elem);
      auto Sij_00 = grad_u(0) + grad_u(0);
      ADReal symmetric_strain_tensor_norm = 0.5 * Utility::pow<2>(Sij_00);
      if (_dim >= 2)
      {
        const auto & grad_v = _v_var->adGradSln(_current_elem);
        auto Sij_01 = grad_u(1) + grad_v(0);
        auto Sij_11 = grad_v(1) + grad_u(1);
        symmetric_strain_tensor_norm +=
            0.5 * (2.0 * Utility::pow<2>(Sij_01) + Utility::pow<2>(Sij_11));
        if (_dim >= 3)
        {
          const auto & grad_w = _w_var->adGradSln(_current_elem);
          auto Sij_02 = grad_u(2) + grad_w(0);
          auto Sij_12 = grad_v(2) + grad_w(1);
          auto Sij_22 = grad_w(2) + grad_w(2);
          symmetric_strain_tensor_norm +=
              0.5 * (2.0 * Utility::pow<2>(Sij_02) + 2.0 * Utility::pow<2>(Sij_12) +
                     Utility::pow<2>(Sij_22));
        }
      }

      auto production_k = _mu_t(makeElemArg(_current_elem)) * symmetric_strain_tensor_norm.value();

      auto time_scale = std::abs(_k(makeElemArg(_current_elem)) / _var(makeElemArg(_current_elem)));

      production = _C1_eps(makeElemArg(_current_elem)) * production_k / time_scale;
      destruction = _C2_eps(makeElemArg(_current_elem)) * _rho(makeElemArg(_current_elem)) *
                    _var(makeElemArg(_current_elem)) / time_scale;

      production = _rf * production + (1.0 - _rf) * _pevious_production[_current_elem];
      destruction = _rf * destruction + (1.0 - _rf) * _pevious_destruction[_current_elem];

      _pevious_production[_current_elem] = production.value();
      _pevious_destruction[_current_elem] = destruction.value();
      _loc_dt = _dt;
    }

    // Implicit relaxation

    // if (_realizable_constraint)
    // {
    //   // Realizable dissipation constraints
    //   production = (production > 0) ? production : 0.0;
    //   destruction = (destruction > 0) ? destruction : 0.0;
    //   destruction = (destruction < production) ? destruction : production;
    // }

    // Solver Relaxation
    // Real diag = 0.0;
    // if (_subproblem.isTransient())
    //   diag += 1.0 / _dt;
    // diag += _mu_t(makeElemArg(_current_elem)).value() *
    // _var.adGradSln(_current_elem).norm().value() /
    //         _var(makeElemArg(_current_elem)).value();

    // residual += 1.0 / _rf * diag * _var(makeElemArg(_current_elem)) -
    //             (1.0 - _rf) / _rf * diag * _var(makeElemArg(_current_elem)).value();

    // Variable Relaxation
    // production = _rf * production + (1.0 - _rf) * production_old_time;
    // destruction = _rf * destruction + (1.0 - _rf) * destruction_old_time;
    // production = _rf * production + (1.0 - _rf) * _production_NL_old[_current_elem];
    // destruction = _rf * destruction + (1.0 - _rf) * _destruction_NL_old[_current_elem];

    // _console << "Production: " << production << std::endl;
    // _console << "Destruction: " << destruction << std::endl;

    // auto new_old_norm = 1.0;
    // if (_fe_problem.getNonlinearSystemBase().getCurrentNonlinearIterationNumber() >= 0)
    // {
    //   new_old_norm = Utility::pow<2>(_var(makeElemArg(_current_elem)).value() -
    //                                  _var(makeElemArg(_current_elem), 1).value()) +
    //                  1e-30;
    // }
    // auto scaling =
    //     std::exp(-1e2 / (1.0 + symmetric_strain_tensor_norm.value()) *
    //              Utility::pow<2>(_var(makeElemArg(_current_elem), 0).value()) / new_old_norm);

    // _console << "-----------------------" << std::endl;
    // _console << "new_old_norm: " << new_old_norm << std::endl;
    // _console << "scaling: " << scaling << std::endl;
    // auto scaling = 0.0;

    // production += std::max(destruction - production, 0.0) * scaling;

    residual += _pevious_destruction[_current_elem] - _pevious_production[_current_elem];

    // residual *= (1.0 - scaling);

    // _console << "Production eps: " << production << " Destruction eps: " << destruction <<
    // std::endl;

    // Updating olds
    // _symmetric_strain_tensor_norm_old[_current_elem] =
    //     _rf * symmetric_strain_tensor_norm.value() +
    //     (1.0 - _rf) * _symmetric_strain_tensor_norm_old[_current_elem];
    // _old_destruction[_current_elem] =
    //     _rf * destruction.value() + (1.0 - _rf) * _old_destruction[_current_elem];

    // residual = -1.0;
    // residual = _var(makeElemArg(_current_elem)) - 1.0;
  }

  _pevious_nl_sol[_current_elem] =
      _rf * _var(makeElemArg(_current_elem)).value() + (1.0 - _rf) * _pevious_nl_sol[_current_elem];
  _production_NL_old[_current_elem] = production.value();
  _destruction_NL_old[_current_elem] = destruction.value();

  return residual;
}
