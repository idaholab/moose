//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVTKEDSourceSink.h"
#include "NonlinearSystemBase.h"
#include "NavierStokesMethods.h"
#include "libmesh/nonlinear_solver.h"

registerMooseObject("NavierStokesApp", INSFVTKEDSourceSink);

InputParameters
INSFVTKEDSourceSink::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription("Elemental kernel to compute the production and destruction "
                             " terms of turbulent kinetic energy dissipation (TKED).");
  params.addRequiredParam<MooseFunctorName>("u", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>("k", "Coupled turbulent kinetic energy.");
  params.deprecateParam("k", NS::TKE, "01/01/2025");
  params.addRequiredParam<MooseFunctorName>(NS::density, "fluid density");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "Dynamic viscosity.");
  params.addRequiredParam<MooseFunctorName>(NS::mu_t, "Turbulent viscosity.");
  params.addParam<std::vector<BoundaryName>>(
      "walls", {}, "Boundaries that correspond to solid walls.");
  params.addParam<bool>(
      "linearized_model",
      true,
      "Boolean to determine if the problem should be used in a linear or nonlinear solve");
  MooseEnum wall_treatment("eq_newton eq_incremental eq_linearized neq", "neq");
  params.addParam<MooseEnum>("wall_treatment",
                             wall_treatment,
                             "The method used for computing the wall functions "
                             "'eq_newton', 'eq_incremental', 'eq_linearized', 'neq'");
  params.addParam<Real>("C1_eps", 1.44, "First epsilon coefficient");
  params.addParam<Real>("C2_eps", 1.92, "Second epsilon coefficient");
  params.addParam<Real>("C_mu", 0.09, "Coupled turbulent kinetic energy closure.");
  params.addParam<Real>("C_pl", 10.0, "Production limiter constant multiplier.");
  params.set<unsigned short>("ghost_layers") = 2;
  params.addParam<bool>("newton_solve", false, "Whether a Newton nonlinear solve is being used");
  params.addParamNamesToGroup("newton_solve", "Advanced");
  return params;
}

INSFVTKEDSourceSink::INSFVTKEDSourceSink(const InputParameters & params)
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
    _wall_treatment(getParam<MooseEnum>("wall_treatment").getEnum<NS::WallTreatmentEnum>()),
    _C1_eps(getParam<Real>("C1_eps")),
    _C2_eps(getParam<Real>("C2_eps")),
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
INSFVTKEDSourceSink::initialSetup()
{
  NS::getWallBoundedElements(
      _wall_boundary_names, _fe_problem, _subproblem, blockIDs(), _wall_bounded);
  NS::getWallDistance(_wall_boundary_names, _fe_problem, _subproblem, blockIDs(), _dist);
  NS::getElementFaceArgs(_wall_boundary_names, _fe_problem, _subproblem, blockIDs(), _face_infos);
}

ADReal
INSFVTKEDSourceSink::computeQpResidual()
{
  ADReal residual = 0.0;
  ADReal production = 0.0;
  ADReal destruction = 0.0;
  const auto elem_arg = makeElemArg(_current_elem);
  const auto state = determineState();
  const auto old_state =
      _linearized_model ? Moose::StateArg(1, Moose::SolutionIterationType::Nonlinear) : state;
  const auto mu = _mu(elem_arg, state);
  const auto rho = _rho(elem_arg, state);
  const auto TKE_old =
      _newton_solve ? std::max(_k(elem_arg, old_state), 1e-10) : _k(elem_arg, old_state);
  ADReal y_plus;

  if (_wall_bounded.find(_current_elem) != _wall_bounded.end())
  {
    std::vector<ADReal> y_plus_vec;

    Real tot_weight = 0.0;

    ADRealVectorValue velocity(_u_var(elem_arg, state));
    if (_v_var)
      velocity(1) = (*_v_var)(elem_arg, state);
    if (_w_var)
      velocity(2) = (*_w_var)(elem_arg, state);

    const auto & face_info_vec = libmesh_map_find(_face_infos, _current_elem);
    const auto & distance_vec = libmesh_map_find(_dist, _current_elem);
    mooseAssert(distance_vec.size(), "Should have found a distance vector");
    mooseAssert(distance_vec.size() == face_info_vec.size(),
                "Should be as many distance vectors as face info vectors");

    for (unsigned int i = 0; i < distance_vec.size(); i++)
    {
      const auto distance = distance_vec[i];
      mooseAssert(distance > 0, "Should be at a non-zero distance");

      if (_wall_treatment == NS::WallTreatmentEnum::NEQ) // Non-equilibrium / Non-iterative
        y_plus = distance * std::sqrt(std::sqrt(_C_mu) * TKE_old) * rho / mu;
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

    for (const auto i : index_range(y_plus_vec))
    {
      const auto y_plus = y_plus_vec[i];

      if (y_plus < 11.25)
      {
        const auto fi = face_info_vec[i];
        const bool defined_on_elem_side = _var.hasFaceSide(*fi, true);
        const Elem * const loc_elem = defined_on_elem_side ? &fi->elem() : fi->neighborPtr();
        const Moose::FaceArg facearg = {
            fi, Moose::FV::LimiterType::CentralDifference, false, false, loc_elem, nullptr};
        destruction += 2.0 * TKE_old * _mu(facearg, state) / rho /
                       Utility::pow<2>(distance_vec[i]) / tot_weight;
      }
      else
        destruction += std::pow(_C_mu, 0.75) * std::pow(TKE_old, 1.5) /
                       (NS::von_karman_constant * distance_vec[i]) / tot_weight;
    }

    residual = _var(makeElemArg(_current_elem), state) - destruction;
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

    ADReal production_k = _mu_t(elem_arg, state) * symmetric_strain_tensor_sq_norm;
    // Compute production limiter (needed for flows with stagnation zones)
    const auto eps_old =
        _newton_solve ? std::max(_var(elem_arg, old_state), 1e-10) : _var(elem_arg, old_state);
    const ADReal production_limit = _C_pl * rho * eps_old;
    // Apply production limiter
    production_k = std::min(production_k, production_limit);

    const auto time_scale = raw_value(TKE_old) / raw_value(eps_old);
    production = _C1_eps * production_k / time_scale;
    destruction = _C2_eps * rho * _var(elem_arg, state) / time_scale;

    residual = destruction - production;
  }

  return residual;
}
