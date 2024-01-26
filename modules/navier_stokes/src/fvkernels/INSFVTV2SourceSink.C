//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVTV2SourceSink.h"
#include "NS.h"
#include "NonlinearSystemBase.h"
#include "NavierStokesMethods.h"
#include "libmesh/nonlinear_solver.h"

registerMooseObject("NavierStokesApp", INSFVTV2SourceSink);

InputParameters
INSFVTV2SourceSink::validParams()
{
  InputParameters params = FVElementalKernel::validParams();

  // Class description
  params.addClassDescription("Elemental kernel to compute the production and destruction "
                             " terms of the wall normal stress (v2).");

  // Velocity fields
  params.addRequiredParam<MooseFunctorName>("u", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", "The velocity in the z direction.");

  // Coupled turbulent variables
  params.addRequiredParam<MooseFunctorName>(NS::TKE, "Coupled turbulent kinetic energy.");
  params.addRequiredParam<MooseFunctorName>(NS::TKED,
                                            "Coupled turbulent kinetic energy dissipation.");
  params.addRequiredParam<MooseFunctorName>(NS::TF, "Coupled turbulent relaxation function.");

  // Coupled thermophysical propeties
  params.addRequiredParam<MooseFunctorName>(NS::density, "fluid density");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "Dynamic viscosity.");
  params.addRequiredParam<MooseFunctorName>(NS::mu_t, "Turbulent viscosity.");

  // Closure parameters
  params.addParam<Real>("C1", 1.4, "First relaxation function coefficient.");
  params.addParam<Real>("C2", 0.3, "Second relaxation function coefficient.");
  params.addParam<Real>("n", 6.0, "Model parameter.");

  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

INSFVTV2SourceSink::INSFVTV2SourceSink(const InputParameters & params)
  : FVElementalKernel(params),
    _dim(_subproblem.mesh().dimension()),
    _u_var(getFunctor<ADReal>("u")),
    _v_var(params.isParamValid("v") ? &(getFunctor<ADReal>("v")) : nullptr),
    _w_var(params.isParamValid("w") ? &(getFunctor<ADReal>("w")) : nullptr),
    _k(getFunctor<ADReal>(NS::TKE)),
    _epsilon(getFunctor<ADReal>(NS::TKED)),
    _f(getFunctor<ADReal>(NS::TF)),
    _rho(getFunctor<ADReal>(NS::density)),
    _mu(getFunctor<ADReal>(NS::mu)),
    _mu_t(getFunctor<ADReal>(NS::mu_t)),
    _C1(getParam<Real>("C1")),
    _C2(getParam<Real>("C2")),
    _n(getParam<Real>("n"))
{
}

ADReal
INSFVTV2SourceSink::computeQpResidual()
{
  // Convinient variables
  const auto elem_arg = makeElemArg(_current_elem);
  const auto state = determineState();
  const auto TKE = _k(elem_arg, state);
  const auto TKED = _epsilon(elem_arg, state);
  const auto TF = _f(elem_arg, state);
  const auto mu = _mu(elem_arg, state);
  const auto rho = _rho(elem_arg, state);
  const auto nu = mu / rho;
  const auto mu_t = _mu_t(elem_arg, state);
  const auto old_state = Moose::StateArg(1, Moose::SolutionIterationType::Nonlinear);
  const auto TV2 = _var(elem_arg, old_state);

  // Computing shrear strain rate deformation tensor
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

  // Computing TKE production and model timescale
  const auto production_TKE = mu_t * symmetric_strain_tensor_sq_norm / rho;
  const auto time_scale = std::max(TKE / TKED, 6 * std::sqrt(nu / TKED));

  // Computing production and destruction
  const auto v2fdiffusivity = ((_C1 - _n) * TV2 - 2.0 / 3.0 * TKE * (_C1 - 1.0)) / time_scale;
  const auto production = std::min(TKE * TF, _C2 * production_TKE - v2fdiffusivity);
  const auto destruction = _n * TV2 / time_scale;

  return rho * (destruction - production);
}
