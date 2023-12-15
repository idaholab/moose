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
  params.addClassDescription("Elemental kernel to compute the production and destruction "
                             " terms of turbulent kinetic energy dissipation (TKED).");
  params.addRequiredCoupledVar("u", "The velocity in the x direction.");
  params.addCoupledVar("v", "The velocity in the y direction.");
  params.addCoupledVar("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>(NS::TKE, "Coupled turbulent kinetic energy.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "fluid density");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "Dynamic viscosity.");
  params.addRequiredParam<MooseFunctorName>(NS::mu_t, "Turbulent viscosity.");
  params.addParam<std::vector<BoundaryName>>(
      "walls", {}, "Boundaries that correspond to solid walls.");
  params.addParam<Real>("max_mixing_length",
                        1e10,
                        "Maximum mixing length allowed for the domain - adjust for realizable "
                        "k-epsilon to work properly.");
  params.addParam<bool>(
      "linearized_model",
      true,
      "Boolean to determine if the problem should be used in a linear or nonlinear solve");
  params.addParam<bool>(
      "non_equilibrium_treatment",
      false,
      "Use non-equilibrium wall treatement (faster than standard wall treatement)");
  params.addParam<Real>("C1_eps", 1.44, "First epsilon coefficient");
  params.addParam<Real>("C2_eps", 1.92, "Second epsilon coefficient");
  params.addParam<Real>("C_mu", 0.09, "Coupled turbulent kinetic energy closure.");

  params.set<unsigned short>("ghost_layers") = 2;
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
    _k(getFunctor<ADReal>(NS::TKE)),
    _rho(getFunctor<ADReal>(NS::density)),
    _mu(getFunctor<ADReal>(NS::mu)),
    _mu_t(getFunctor<ADReal>(NS::mu_t)),
    _wall_boundary_names(getParam<std::vector<BoundaryName>>("walls")),
    _max_mixing_length(getParam<Real>("max_mixing_length")),
    _linearized_model(getParam<bool>("linearized_model")),
    _non_equilibrium_treatment(getParam<bool>("non_equilibrium_treatment")),
    _C1_eps(getParam<Real>("C1_eps")),
    _C2_eps(getParam<Real>("C2_eps")),
    _C_mu(getParam<Real>("C_mu"))
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
  const Moose::StateArg state = determineState();
  const auto old_state =
      _linearized_model ? Moose::StateArg(1, Moose::SolutionIterationType::Nonlinear) : state;
  const auto mu = _mu(makeElemArg(_current_elem), state);
  const auto rho = _rho(makeElemArg(_current_elem), state);
  const auto TKE = _k(makeElemArg(_current_elem), state);

  if (_wall_bounded[_current_elem])
  {
    std::vector<ADReal> y_plus_vec;

    Real tot_weight = 0.0;

    ADRealVectorValue velocity(_u_var->getElemValue(_current_elem, state));
    if (_v_var)
      velocity(1) = _v_var->getElemValue(_current_elem, state);
    if (_w_var)
      velocity(2) = _w_var->getElemValue(_current_elem, state);

    for (unsigned int i = 0; i < _dist[_current_elem].size(); i++)
    {
      const auto distance = _dist[_current_elem][i];

      ADReal y_plus;
      if (_non_equilibrium_treatment)
        y_plus = std::pow(_C_mu, 0.25) * distance * std::sqrt(TKE) / mu;
      else
      {
        const auto parallel_speed = (velocity - velocity * _face_infos[_current_elem][i]->normal() *
                                                    _face_infos[_current_elem][i]->normal())
                                        .norm();

        y_plus = NS::findyPlus(mu, rho, std::max(parallel_speed, 1e-10), distance);
      }

      y_plus_vec.push_back(y_plus);

      tot_weight += 1.0;
    }

    for (unsigned int i = 0; i < y_plus_vec.size(); i++)
    {
      const auto y_plus = y_plus_vec[i];

      if (y_plus < 11.25)
      {
        const auto fi = _face_infos[_current_elem][i];
        const bool defined_on_elem_side = _var.hasFaceSide(*fi, true);
        const Elem * const loc_elem = defined_on_elem_side ? &fi->elem() : fi->neighborPtr();
        const Moose::FaceArg facearg = {
            fi, Moose::FV::LimiterType::CentralDifference, false, false, loc_elem};
        const ADReal wall_mut = _mu_t(facearg, state);
        destruction += 2.0 * TKE * wall_mut / Utility::pow<2>(_dist[_current_elem][i]) / tot_weight;
      }
      else
        destruction += std::pow(_C_mu, 0.75) * rho * std::pow(TKE, 1.5) /
                       (NS::von_karman_constant * _dist[_current_elem][i]) / tot_weight;
    }

    residual = _var(makeElemArg(_current_elem), state) - destruction;
  }
  else
  {
    const auto & grad_u = _u_var->adGradSln(_current_elem, state);
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
      const auto & grad_v = _v_var->adGradSln(_current_elem, state);
      Sij_xy = grad_u(1) + grad_v(0);
      Sij_yy = 2.0 * grad_v(1);

      grad_xy = grad_u(1);
      grad_yx = grad_v(0);
      grad_yy = grad_v(1);

      trace += Sij_yy / 3.0;

      if (_dim >= 3)
      {
        const auto & grad_w = _w_var->adGradSln(_current_elem, state);

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

    const auto production_k = _C_mu * symmetric_strain_tensor_sq_norm * TKE;

    production = _C1_eps * rho * production_k;

    const auto time_scale = _k(makeElemArg(_current_elem), old_state) /
                                (_var(makeElemArg(_current_elem), old_state) + 1e-15) +
                            1e-15;

    destruction = _C2_eps * rho * _var(makeElemArg(_current_elem), state) / time_scale;

    // Production limiter - not needed for most applications
    if (_max_mixing_length < 1e10)
      if (std::pow(std::abs(production), 1.5) / std::abs(destruction) > _max_mixing_length)
        production = std::pow(_max_mixing_length * std::abs(destruction.value()) + 1e-10, 2. / 3.);

    residual = destruction - production;
  }

  return residual;
}
