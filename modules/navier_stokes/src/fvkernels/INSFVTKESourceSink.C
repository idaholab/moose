//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVTKESourceSink.h"
#include "NonlinearSystemBase.h"
#include "NavierStokesMethods.h"
#include "libmesh/nonlinear_solver.h"

registerMooseObject("NavierStokesApp", INSFVTKESourceSink);

InputParameters
INSFVTKESourceSink::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription("Elemental kernel to compute the production and destruction "
                             " terms of turbulent kinetic energy (TKE).");
  params.addRequiredParam<MooseFunctorName>("u", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>(NS::TKED,
                                            "Coupled turbulent kinetic energy dissipation rate.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "fluid density");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "Dynamic viscosity.");
  params.addRequiredParam<MooseFunctorName>(NS::mu_t, "Turbulent viscosity.");
  params.addParam<std::vector<BoundaryName>>(
      "walls", {}, "Boundaries that correspond to solid walls.");
  params.addParam<bool>(
      "linearized_model",
      true,
      "Boolean to determine if the problem should be use in a linear or nonlinear solve.");
  MooseEnum wall_treatment("eq_newton eq_incremental eq_linearized neq", "neq");
  params.addParam<MooseEnum>("wall_treatment",
                             wall_treatment,
                             "The method used for computing the wall functions "
                             "'eq_newton', 'eq_incremental', 'eq_linearized', 'neq'");
  params.addParam<Real>("C_mu", 0.09, "Coupled turbulent kinetic energy closure.");
  params.addParam<Real>("C_pl", 10.0, "Production Limiter Constant Multiplier.");
  params.set<unsigned short>("ghost_layers") = 2;
  params.addParam<bool>("newton_solve", false, "Whether a Newton nonlinear solve is being used");
  params.addParamNamesToGroup("newton_solve", "Advanced");

  return params;
}

INSFVTKESourceSink::INSFVTKESourceSink(const InputParameters & params)
  : FVElementalKernel(params),
    _dim(_subproblem.mesh().dimension()),
    _u_var(getFunctor<ADReal>("u")),
    _v_var(params.isParamValid("v") ? &(getFunctor<ADReal>("v")) : nullptr),
    _w_var(params.isParamValid("w") ? &(getFunctor<ADReal>("w")) : nullptr),
    _epsilon(getFunctor<ADReal>(NS::TKED)),
    _rho(getFunctor<ADReal>(NS::density)),
    _mu(getFunctor<ADReal>(NS::mu)),
    _mu_t(getFunctor<ADReal>(NS::mu_t)),
    _wall_boundary_names(getParam<std::vector<BoundaryName>>("walls")),
    _linearized_model(getParam<bool>("linearized_model")),
    _wall_treatment(getParam<MooseEnum>("wall_treatment").getEnum<NS::WallTreatmentEnum>()),
    _C_mu(getParam<Real>("C_mu")),
    _C_pl(getParam<Real>("C_pl")),
    _newton_solve(getParam<bool>("newton_solve"))
{
  if (_dim >= 2 && !_v_var)
    paramError("v", "In two or more dimensions, the v velocity must be supplied!");

  if (_dim >= 3 && !_w_var)
    paramError("w", "In three or more dimensions, the w velocity must be supplied!");
}

void
INSFVTKESourceSink::initialSetup()
{
  NS::getWallBoundedElements(
      _wall_boundary_names, _fe_problem, _subproblem, blockIDs(), _wall_bounded);
  NS::getWallDistance(_wall_boundary_names, _fe_problem, _subproblem, blockIDs(), _dist);
  NS::getElementFaceArgs(_wall_boundary_names, _fe_problem, _subproblem, blockIDs(), _face_infos);
}

ADReal
INSFVTKESourceSink::computeQpResidual()
{
  ADReal residual = 0.0;
  ADReal production = 0.0;
  ADReal destruction = 0.0;

  const auto state = determineState();
  const auto elem_arg = makeElemArg(_current_elem);
  const auto old_state =
      _linearized_model ? Moose::StateArg(1, Moose::SolutionIterationType::Nonlinear) : state;
  const auto rho = _rho(elem_arg, state);
  const auto mu = _mu(elem_arg, state);
  // To prevent negative values & preserve sparsity pattern
  auto TKE = _newton_solve
                 ? std::max(_var(elem_arg, old_state), ADReal(0) * _var(elem_arg, old_state))
                 : _var(elem_arg, old_state);
  // Prevent computation of sqrt(0) with undefined automatic derivatives
  // This is not needed for segregated solves, as TKE has minimum bound in the solver
  if (_newton_solve)
    TKE = std::max(TKE, 1e-10);

  if (_wall_bounded.find(_current_elem) != _wall_bounded.end())
  {
    std::vector<ADReal> y_plus_vec, velocity_grad_norm_vec;

    Real tot_weight = 0.0;

    ADRealVectorValue velocity(_u_var(elem_arg, state));
    if (_v_var)
      velocity(1) = (*_v_var)(elem_arg, state);
    if (_w_var)
      velocity(2) = (*_w_var)(elem_arg, state);

    const auto & face_info_vec = libmesh_map_find(_face_infos, _current_elem);
    const auto & distance_vec = libmesh_map_find(_dist, _current_elem);

    for (unsigned int i = 0; i < distance_vec.size(); i++)
    {
      const auto parallel_speed = NS::computeSpeed(
          velocity - velocity * face_info_vec[i]->normal() * face_info_vec[i]->normal());
      const auto distance = distance_vec[i];

      ADReal y_plus;
      if (_wall_treatment == NS::WallTreatmentEnum::NEQ) // Non-equilibrium / Non-iterative
        y_plus = distance * std::sqrt(std::sqrt(_C_mu) * TKE) * rho / mu;
      else // Equilibrium / Iterative
        y_plus = NS::findyPlus(mu, rho, std::max(parallel_speed, 1e-10), distance);

      y_plus_vec.push_back(y_plus);

      const ADReal velocity_grad_norm = parallel_speed / distance;

      /// Do not erase!!
      // More complete expansion for velocity gradient. Leave commented for now.
      // Will be useful later when doing two-phase or compressible flow
      // ADReal velocity_grad_norm_sq =
      //     Utility::pow<2>(_u_var->gradient(elem_arg, state) *
      //                     _normal[_current_elem][i]);
      // if (_dim >= 2)
      //   velocity_grad_norm_sq +=
      //       Utility::pow<2>(_v_var->gradient(elem_arg, state) *
      //                       _normal[_current_elem][i]);
      // if (_dim >= 3)
      //   velocity_grad_norm_sq +=
      //       Utility::pow<2>(_w_var->gradient(elem_arg, state) *
      //                       _normal[_current_elem][i]);
      // ADReal velocity_grad_norm = std::sqrt(velocity_grad_norm_sq);

      velocity_grad_norm_vec.push_back(velocity_grad_norm);

      tot_weight += 1.0;
    }

    for (unsigned int i = 0; i < y_plus_vec.size(); i++)
    {
      const auto y_plus = y_plus_vec[i];

      const auto fi = face_info_vec[i];
      const bool defined_on_elem_side = _var.hasFaceSide(*fi, true);
      const Elem * const loc_elem = defined_on_elem_side ? &fi->elem() : fi->neighborPtr();
      const Moose::FaceArg facearg = {
          fi, Moose::FV::LimiterType::CentralDifference, false, false, loc_elem, nullptr};
      const ADReal wall_mut = _mu_t(facearg, state);
      const ADReal wall_mu = _mu(facearg, state);

      const auto destruction_visc = 2.0 * wall_mu / Utility::pow<2>(distance_vec[i]) / tot_weight;
      const auto destruction_log = std::pow(_C_mu, 0.75) * rho * std::pow(TKE, 0.5) /
                                   (NS::von_karman_constant * distance_vec[i]) / tot_weight;
      const auto tau_w = (wall_mut + wall_mu) * velocity_grad_norm_vec[i];

      // Additional 0-value terms to make sure new derivative entries are not added during the solve
      if (y_plus < 11.25)
      {
        destruction += destruction_visc;
        if (_newton_solve)
          destruction += 0 * destruction_log + 0 * tau_w;
      }
      else
      {
        destruction += destruction_log;
        if (_newton_solve)
          destruction += 0 * destruction_visc;
        production += tau_w * std::pow(_C_mu, 0.25) / std::sqrt(TKE) /
                      (NS::von_karman_constant * distance_vec[i]) / tot_weight;
      }
    }

    residual = (destruction - production) * _var(elem_arg, state);
    // Additional 0-value term to make sure new derivative entries are not added during the solve
    if (_newton_solve)
      residual += 0 * _epsilon(elem_arg, old_state);
  }
  else
  {
    const auto & grad_u = _u_var.gradient(elem_arg, state);
    const auto Sij_xx = 2.0 * grad_u(0);
    ADReal Sij_xy = 0.0;
    ADReal Sij_xz = 0.0;
    ADReal Sij_yy = 0.0;
    ADReal Sij_yz = 0.0;
    ADReal Sij_zz = 0.0;

    const auto grad_xx = grad_u(0);
    ADReal grad_xy = 0.0;
    ADReal grad_xz = 0.0;
    ADReal grad_yx = 0.0;
    ADReal grad_yy = 0.0;
    ADReal grad_yz = 0.0;
    ADReal grad_zx = 0.0;
    ADReal grad_zy = 0.0;
    ADReal grad_zz = 0.0;

    auto trace = Sij_xx / 3.0;

    if (_dim >= 2)
    {
      const auto & grad_v = (*_v_var).gradient(elem_arg, state);
      Sij_xy = grad_u(1) + grad_v(0);
      Sij_yy = 2.0 * grad_v(1);

      grad_xy = grad_u(1);
      grad_yx = grad_v(0);
      grad_yy = grad_v(1);

      trace += Sij_yy / 3.0;

      if (_dim >= 3)
      {
        const auto & grad_w = (*_w_var).gradient(elem_arg, state);

        Sij_xz = grad_u(2) + grad_w(0);
        Sij_yz = grad_v(2) + grad_w(1);
        Sij_zz = 2.0 * grad_w(2);

        grad_xz = grad_u(2);
        grad_yz = grad_v(2);
        grad_zx = grad_w(0);
        grad_zy = grad_w(1);
        grad_zz = grad_w(2);

        trace += Sij_zz / 3.0;
      }
    }

    const auto symmetric_strain_tensor_sq_norm =
        (Sij_xx - trace) * grad_xx + Sij_xy * grad_xy + Sij_xz * grad_xz + Sij_xy * grad_yx +
        (Sij_yy - trace) * grad_yy + Sij_yz * grad_yz + Sij_xz * grad_zx + Sij_yz * grad_zy +
        (Sij_zz - trace) * grad_zz;

    production = _mu_t(elem_arg, state) * symmetric_strain_tensor_sq_norm;

    const auto tke_old_raw = raw_value(TKE);
    const auto epsilon_old = _epsilon(elem_arg, old_state);

    if (MooseUtils::absoluteFuzzyEqual(tke_old_raw, 0))
      destruction = rho * epsilon_old;
    else
      destruction = rho * _var(elem_arg, state) / tke_old_raw * raw_value(epsilon_old);

    // k-Production limiter (needed for flows with stagnation zones)
    const ADReal production_limit =
        _C_pl * rho * (_newton_solve ? std::max(epsilon_old, ADReal(0)) : epsilon_old);

    // Apply production limiter
    production = std::min(production, production_limit);

    residual = destruction - production;

    // Additional 0-value terms to make sure new derivative entries are not added during the solve
    if (_newton_solve)
      residual += 0 * _epsilon(elem_arg, state);
  }

  return residual;
}
