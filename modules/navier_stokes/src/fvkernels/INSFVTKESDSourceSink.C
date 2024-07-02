//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVTKESDSourceSink.h"
#include "NS.h"
#include "NonlinearSystemBase.h"
#include "NavierStokesMethods.h"
#include "libmesh/nonlinear_solver.h"

registerMooseObject("NavierStokesApp", INSFVTKESDSourceSink);

InputParameters
INSFVTKESDSourceSink::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription("Elemental kernel to compute the production and destruction "
                             " terms of turbulent kinetic energy dissipation (TKESD).");
  params.addRequiredParam<MooseFunctorName>("u", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>(NS::TKE, "Coupled turbulent kinetic energy.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "fluid density");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "Dynamic viscosity.");
  params.addRequiredParam<MooseFunctorName>(NS::mu_t, "Turbulent viscosity.");
  params.addParam<std::vector<BoundaryName>>(
      "walls", {}, "Boundaries that correspond to solid walls.");
  params.addParam<bool>(
      "linearized_model",
      true,
      "Boolean to determine if the problem should be used in a linear or nonlinear solve");
  MooseEnum wall_treatment("eq_newton eq_incremental eq_linearized neq", "eq_newton");
  params.addParam<MooseEnum>("wall_treatment",
                             wall_treatment,
                             "The method used for computing the wall functions "
                             "'eq_newton', 'eq_incremental', 'eq_linearized', 'neq'");
  params.addRequiredParam<MooseFunctorName>("F1", "The F1 blending function.");
  params.addParam<bool>("free_shear_modification",
                        false,
                        "Activate free shear modification for the turbulent kinetic energy.");
  params.addParam<MooseFunctorName>("wall_normal_unit_vectors",
                                    "The diagonal coefficients of a diffusion tensor.");
  params.addParam<bool>(
      "vortex_stretching_modficiation",
      false,
      "Activate vortex stretching modification for the turbulent kinetic energy.");
  params.addParam<bool>(
      "low_Re_modification", false, "Activate low Reynolds number modifications.");
  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

INSFVTKESDSourceSink::INSFVTKESDSourceSink(const InputParameters & params)
  : FVElementalKernel(params),
    _dim(_subproblem.mesh().dimension()),
    _u_var(getFunctor<ADReal>("u")),
    _v_var(params.isParamValid("v") ? &(getFunctor<ADReal>("v")) : nullptr),
    _w_var(params.isParamValid("w") ? &(getFunctor<ADReal>("w")) : nullptr),
    _k(getFunctor<ADReal>(NS::TKE)),
    _rho(getFunctor<ADReal>(NS::density)),
    _mu(getFunctor<ADReal>(NS::mu)),
    _mu_t(getFunctor<ADReal>(NS::mu_t)),
    _wall_boundary_names(getParam<std::vector<BoundaryName>>("walls")),
    _linearized_model(getParam<bool>("linearized_model")),
    _wall_treatment(getParam<MooseEnum>("wall_treatment")),
    _F1(getFunctor<ADReal>("F1")),
    _bool_free_shear_modficiation(getParam<bool>("free_shear_modification")),
    _wall_normal_unit_vectors(params.isParamValid("wall_normal_unit_vectors")
                                  ? &(getFunctor<ADRealVectorValue>("wall_normal_unit_vectors"))
                                  : nullptr),
    _bool_vortex_stretching_modficiation(getParam<bool>("vortex_stretching_modficiation")),
    _bool_low_Re_modification(getParam<bool>("low_Re_modification"))
{
  if (_dim >= 2 && !_v_var)
    paramError("v", "In two or more dimensions, the v velocity must be supplied!");

  if (_dim >= 3 && !_w_var)
    paramError("w", "In three or more dimensions, the w velocity must be supplied!");

  if (_bool_free_shear_modficiation && !(_wall_normal_unit_vectors))
    paramError(
        "wall_normal_unit_vectors",
        "A map with `wall_normal_unit_vectors` should be provided for activating the free shear "
        "modification. Consider using WallNormalUnitVectorAux for computing this map.");
}

void
INSFVTKESDSourceSink::initialSetup()
{
  NS::getWallBoundedElements(
      _wall_boundary_names, _fe_problem, _subproblem, blockIDs(), _wall_bounded);
  NS::getWallDistance(_wall_boundary_names, _fe_problem, _subproblem, blockIDs(), _dist);
  NS::getElementFaceArgs(_wall_boundary_names, _fe_problem, _subproblem, blockIDs(), _face_infos);
}

ADReal
INSFVTKESDSourceSink::computeQpResidual()
{
  // Variables to build up the residual
  ADReal residual = 0.0;
  ADReal production = 0.0;
  ADReal destruction = 0.0;

  // Convenient variables
  const auto elem_arg = makeElemArg(_current_elem);
  const auto state = determineState();
  const auto old_state =
      _linearized_model ? Moose::StateArg(1, Moose::SolutionIterationType::Nonlinear) : state;
  const auto mu = _mu(elem_arg, state);
  const auto rho = _rho(elem_arg, state);
  const auto TKE = _k(elem_arg, old_state);

  // Wall bounded element formulation - setting the value of omega
  if (_wall_bounded.find(_current_elem) != _wall_bounded.end())
  {
    std::vector<ADReal> y_plus_vec;
    ADReal y_plus;
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
      const auto distance = distance_vec[i];

      if (_wall_treatment == "neq")
      {
        // Non-equilibrium / Non-iterative
        y_plus = distance * std::sqrt(std::sqrt(_C_mu) * TKE) * rho / mu;
      }
      else
      {
        // Equilibrium / Iterative
        const auto parallel_speed = NS::computeSpeed(
            velocity - velocity * face_info_vec[i]->normal() * face_info_vec[i]->normal());

        y_plus = NS::findyPlus(mu, rho, std::max(parallel_speed, 1e-10), distance);
      }

      y_plus_vec.push_back(y_plus);

      tot_weight += 1.0;
    }

    for (unsigned int i = 0; i < y_plus_vec.size(); i++)
    {
      // Convenient variables
      const auto y_plus = y_plus_vec[i];
      const auto u_tau_2 = std::sqrt(_C_mu) * TKE;
      const auto fi = face_info_vec[i];
      const bool defined_on_elem_side = _var.hasFaceSide(*fi, true);
      const Elem * const loc_elem = defined_on_elem_side ? &fi->elem() : fi->neighborPtr();
      const Moose::FaceArg facearg = {
          fi, Moose::FV::LimiterType::CentralDifference, false, false, loc_elem};

      // Blended wall functions
      const auto omegaVis = 6.0 * _mu(facearg, state) /
                            (_rho(facearg, state) * _beta_i_1 * Utility::pow<2>(distance_vec[i]));
      const auto omegaLog =
          std::sqrt(TKE) / (std::pow(_C_mu, 0.25) * NS::von_karman_constant * distance_vec[i]);
      const auto Re_d =
          std::sqrt(_var(elem_arg, old_state)) * distance_vec[i] * rho / _mu(facearg, state);
      const auto gamma = std::exp(-Re_d / 11.0);
      destruction += gamma * omegaVis + (1.0 - gamma) * omegaLog;
    }

    residual = _var(makeElemArg(_current_elem), state) - destruction;
  }
  // Bulk formulation
  else
  {
    // ***** Computing shear strain rate ***** //
    // *************************************** //
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
    // *************************************** //

    // Low Reynolds number modifications to production
    const auto F1 = _F1(elem_arg, old_state);
    ADReal gamma;
    if (_bool_low_Re_modification)
    {
      const auto omega_capped = std::max(_var(elem_arg, old_state), 1e-12);
      const auto Re_shear = rho * TKE / (mu * omega_capped);
      const auto Re_ratio_omega = Re_shear / _Re_omega;

      const auto beta = F1 * _beta_i_1 + (1.0 - F1) * _beta_i_2;
      const auto alpha_1_star = ((beta / 3.0) + Re_shear / _Re_k) / (1.0 + Re_shear / _Re_k);
      const auto alpha_star = F1 * alpha_1_star + (1.0 - F1) * _alpha_2_star;

      const auto gamma_1 =
          _gamma_infty_1 * (_alpha_0 + Re_ratio_omega) / (1.0 + Re_ratio_omega) / alpha_star;

      gamma = F1 * gamma_1 + (1.0 - F1) * _gamma_infty_2;
    }
    else
      gamma = F1 * _gamma_infty_1 + (1.0 - F1) * _gamma_infty_2;

    // TKSD production
    production = rho * gamma * symmetric_strain_tensor_sq_norm;

    // Vortex stretching modification to TKSE destruction
    ADReal f_beta(1.0);
    if (_bool_vortex_stretching_modficiation)
    {
      const auto grad_u = _u_var.gradient(elem_arg, state);
      TensorValue<ADReal> grad_vel_vec(grad_u(0));
      if (_dim >= 2)
      {
        const auto grad_v = (*_v_var).gradient(elem_arg, state);
        grad_vel_vec(0, 1) = grad_u(1);
        grad_vel_vec(1, 0) = grad_v(0);
        grad_vel_vec(1, 1) = grad_v(1);
        if (_dim >= 3)
        {
          const auto grad_w = (*_w_var).gradient(elem_arg, state);
          grad_vel_vec(0, 2) = grad_u(2);
          grad_vel_vec(1, 2) = grad_v(2);
          grad_vel_vec(2, 0) = grad_w(0);
          grad_vel_vec(2, 1) = grad_w(1);
          grad_vel_vec(2, 2) = grad_w(2);
        }
      }
      const auto S = 0.5 * (grad_vel_vec + grad_vel_vec.transpose());
      const auto W = 0.5 * (grad_vel_vec - grad_vel_vec.transpose());
      const auto stretch_rate_ind =
          (W * W).contract(S) /
          Utility::pow<3>(_beta_infty * std::max(_var(elem_arg, old_state), 1e-12));
      if (_bool_free_shear_modficiation)
        f_beta = (1.0 + 85.0 * stretch_rate_ind) / (1.0 + 100.0 * stretch_rate_ind);
      else
        f_beta = (1.0 + 70.0 * stretch_rate_ind) / (1.0 + 80.0 * stretch_rate_ind);
    }

    // Destruction
    const auto beta_star = F1 * _beta_i_1 + (1.0 - F1) * _beta_i_2;
    destruction = rho * beta_star * f_beta * _var(elem_arg, old_state) * _var(elem_arg, state);

    // Cross diffusion term
    const auto & grad_k = _k.gradient(elem_arg, old_state);
    const auto & grad_omega = _var.gradient(elem_arg, old_state);
    auto cross_diffusion = grad_k(0) * grad_omega(0);
    if (_dim > 1)
      cross_diffusion += grad_k(1) * grad_omega(1);
    if (_dim > 2)
      cross_diffusion += grad_k(2) * grad_omega(2);
    cross_diffusion *=
        2.0 * (1.0 - F1) * rho / _sigma_omega_2 / std::max(_var(elem_arg, old_state), 1e-12);

    // Free shear modification to cross diffusion
    if (_bool_free_shear_modficiation)
    {
      const auto cell_wall_normal = (*_wall_normal_unit_vectors)(elem_arg, state);
      auto cross_wall_diffusion =
          (grad_k(0) * cell_wall_normal(0)) * (grad_omega(0) * cell_wall_normal(0));
      if (_dim > 1)
        cross_wall_diffusion +=
            (grad_k(1) * cell_wall_normal(1)) * (grad_omega(1) * cell_wall_normal(1));
      if (_dim > 2)
        cross_wall_diffusion +=
            (grad_k(2) * cell_wall_normal(2)) * (grad_omega(2) * cell_wall_normal(2));
      if (cross_wall_diffusion > 0)
        cross_diffusion +=
            0.125 * rho / std::max(_var(elem_arg, old_state), 1e-12) * cross_wall_diffusion;
    }

    // Computing residual
    residual = destruction - production - cross_diffusion;
  }

  return residual;
}
