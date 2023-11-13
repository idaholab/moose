//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVTKESourceSink.h"
#include "NS.h"
#include "NonlinearSystemBase.h"
#include "NavierStokesMethods.h"
#include "libmesh/nonlinear_solver.h"

registerMooseObject("NavierStokesApp", INSFVTKESourceSink);

InputParameters
INSFVTKESourceSink::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription("Elental kernel to compute the production and destruction "
                             " terms of turbulent kinetic energy (TKE).");
  params.addRequiredCoupledVar("u", "The velocity in the x direction.");
  params.addCoupledVar("v", "The velocity in the y direction.");
  params.addCoupledVar("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>("epsilon",
                                            "Coupled turbulent kinetic energy dissipation rate.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "fluid density");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "Dynamic viscosity.");
  params.addRequiredParam<MooseFunctorName>("mu_t", "Turbulent viscosity.");
  params.addParam<std::vector<BoundaryName>>("walls", "Boundaries that correspond to solid walls.");

  params.addParam<Real>("max_mixing_length",
                        1e10,
                        "Maximum mixing legth allowed for the domain - adjust for realizable "
                        "k-epsilon to work properly.");
  params.addParam<bool>(
      "linearized_model",
      true,
      "Boolean to determine if the problem should be use in a linear or nonlinear solve.");
  params.addParam<bool>(
      "non_equilibrium_treatement",
      false,
      "Use non-equilibrium wall treatement (faster than standard wall treatement)");
  params.addParam<Real>("C_mu", 0.09, "Coupled turbulent kinetic energy closure.");

  params.set<unsigned short>("ghost_layers") = 2;

  return params;
}

INSFVTKESourceSink::INSFVTKESourceSink(const InputParameters & params)
  : FVElementalKernel(params),
    _dim(_subproblem.mesh().dimension()),
    _u_var(dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("u", 0))),
    _v_var(params.isParamValid("v")
               ? dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("v", 0))
               : nullptr),
    _w_var(params.isParamValid("w")
               ? dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("w", 0))
               : nullptr),
    _epsilon(getFunctor<ADReal>("epsilon")),
    _rho(getFunctor<ADReal>(NS::density)),
    _mu(getFunctor<ADReal>(NS::mu)),
    _mu_t(getFunctor<ADReal>("mu_t")),
    _wall_boundary_names(getParam<std::vector<BoundaryName>>("walls")),
    _max_mixing_length(getParam<Real>("max_mixing_length")),
    _linearized_model(getParam<bool>("linearized_model")),
    _non_equilibrium_treatement(getParam<bool>("non_equilibrium_treatement")),
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

  _wall_bounded = *(NS::getWallBoundedElements(_wall_boundary_names, _fe_problem, _subproblem));
  _normal = *(NS::getElementFaceNormal(_wall_boundary_names, _fe_problem, _subproblem));
  _dist = *(NS::getWallDistance(_wall_boundary_names, _fe_problem, _subproblem));
  _face_infos = *(NS::getElementFaceArgs(_wall_boundary_names, _fe_problem, _subproblem));
}

ADReal
INSFVTKESourceSink::computeQpResidual()
{

  ADReal residual = 0.0;
  ADReal production = 0.0;
  ADReal destruction = 0.0;
  const Moose::StateArg state = determineState();
  auto old_state =
      _linearized_model ? Moose::StateArg(1, Moose::SolutionIterationType::Nonlinear) : state;

  if (_wall_bounded[_current_elem])
  {
    std::vector<ADReal> y_plus_vec, velocity_grad_norm_vec;

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

      auto y_plus = NS::findyPlus(_mu(makeElemArg(_current_elem), state),
                                  _rho(makeElemArg(_current_elem), state),
                                  std::max(parallel_speed, 1e-10),
                                  distance);

      y_plus_vec.push_back(y_plus);

      ADReal velocity_grad_norm = parallel_speed / _dist[_current_elem][i];

      /// Do not erase!!
      /// More complete expansion for velocity gradient. Leave commented for now.
      //// Will be useful later when doing two-phase or compressible flow
      // ADReal velocity_grad_norm_sq =
      //     Utility::pow<2>(_u_var->gradient(makeElemArg(_current_elem), state) *
      //                     _normal[_current_elem][i]);

      // if (_dim >= 2)
      //   velocity_grad_norm_sq +=
      //       Utility::pow<2>(_v_var->gradient(makeElemArg(_current_elem), state) *
      //                       _normal[_current_elem][i]);

      // if (_dim >= 3)
      //   velocity_grad_norm_sq +=
      //       Utility::pow<2>(_w_var->gradient(makeElemArg(_current_elem), state) *
      //                       _normal[_current_elem][i]);

      // ADReal velocity_grad_norm = std::sqrt(velocity_grad_norm_sq);

      velocity_grad_norm_vec.push_back(velocity_grad_norm);

      tot_weight += 1.0;
    }

    for (unsigned int i = 0; i < y_plus_vec.size(); i++)
    {
      auto y_plus = y_plus_vec[i];

      auto fi = _face_infos[_current_elem][i];
      const bool defined_on_elem_side = _var.hasFaceSide(*fi, true);
      const Elem * const loc_elem = defined_on_elem_side ? &fi->elem() : fi->neighborPtr();
      Moose::FaceArg facearg = {
          fi, Moose::FV::LimiterType::CentralDifference, false, false, loc_elem};
      ADReal wall_mut = _mu_t(facearg, state);

      auto destruction_visc =
          2.0 * wall_mut / Utility::pow<2>(_dist[_current_elem][i]) / tot_weight;
      auto destruction_log = std::pow(_C_mu, 0.75) * _rho(makeElemArg(_current_elem), state) *
                             std::pow(_var(makeElemArg(_current_elem), old_state), 0.5) /
                             (_von_karman * _dist[_current_elem][i]) / tot_weight;

      if (y_plus < 11.25)
        destruction += destruction_visc;
      else
        destruction += destruction_log;

      production += wall_mut * velocity_grad_norm_vec[i] * std::pow(_C_mu, 0.25) /
                    std::sqrt(_var(makeElemArg(_current_elem), old_state) + 1e-10) /
                    (_von_karman * _dist[_current_elem][i]) / tot_weight;
    }

    residual = (destruction - production) * _var(makeElemArg(_current_elem), state);
  }
  else
  {

    const auto & grad_u = _u_var->adGradSln(_current_elem, state);
    auto Sij_xx = 2.0 * grad_u(0);
    ADReal Sij_xy = 0.0;
    ADReal Sij_xz = 0.0;
    ADReal Sij_yy = 0.0;
    ADReal Sij_yz = 0.0;
    ADReal Sij_zz = 0.0;

    auto grad_xx = grad_u(0);
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

    auto symmetric_strain_tensor_sq_norm =
        (Sij_xx - trace) * grad_xx + Sij_xy * grad_xy + Sij_xz * grad_xz + Sij_xy * grad_yx +
        (Sij_yy - trace) * grad_yy + Sij_yz * grad_yz + Sij_xz * grad_zx + Sij_yz * grad_zy +
        (Sij_zz - trace) * grad_zz;

    production = _mu_t(makeElemArg(_current_elem), state) * symmetric_strain_tensor_sq_norm;

    auto time_scale = raw_value(_var(makeElemArg(_current_elem), old_state) /
                                    (_epsilon(makeElemArg(_current_elem), old_state) + 1e-15) +
                                1e-15);

    destruction = _rho(makeElemArg(_current_elem), state) *
                  _var(makeElemArg(_current_elem), state) / time_scale;

    // Production limiter - not needed for most applications
    if (_max_mixing_length < 1e10)
    {
      if (std::pow(std::abs(production), 1.5) / std::abs(destruction) > _max_mixing_length)
        production = std::pow(_max_mixing_length * std::abs(destruction.value()) + 1e-10, 2. / 3.);
    }

    residual = destruction - production;
  }

  return residual;
}
